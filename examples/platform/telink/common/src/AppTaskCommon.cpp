/*
 *
 *    Copyright (c) 2022-2024 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "AppTaskCommon.h"
#include "AppTask.h"
#if CONFIG_SOC_RISCV_TELINK_B92 || CONFIG_SOC_RISCV_TELINK_TL321X
#include <analog_user.h>
#endif

#include "BLEManagerImpl.h"
#include "ButtonManager.h"
#include "LEDManager.h"
#include "PWMManager.h"

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "ThreadUtil.h"
#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/Zephyr/InetUtils.h>
#include <platform/telink/wifi/TelinkWiFiDriver.h>
#endif

#include <DeviceInfoProviderImpl.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/persistence/DefaultAttributePersistenceProvider.h>
#include <app/util/persistence/DeferredAttributePersistenceProvider.h>
#include <data-model-providers/codegen/Instance.h>
#include <setup_payload/OnboardingCodesUtil.h>

#if CONFIG_BOOTLOADER_MCUBOOT
#include <OTAUtil.h>
#endif

#if CONFIG_CHIP_OTA_REQUESTOR
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#endif

#include <app-common/zap-generated/attributes/Accessors.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/settings/settings.h>

#if CONFIG_DUAL_MODE_SWTICH
uint8_t sBoot_zb       = 0;
uint8_t para_lightness = 0;
user_para_t user_para;
light_para_t light_para;
#endif

using namespace chip::app;

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace {
constexpr int kFactoryResetCalcTimeout = 3000;
constexpr int kFactoryResetTriggerCntr = 3;
constexpr int kAppEventQueueSize       = 10;

constexpr uint32_t kIdentifyBlinkRateMs         = 200;
constexpr uint32_t kIdentifyOkayOnRateMs        = 50;
constexpr uint32_t kIdentifyOkayOffRateMs       = 950;
constexpr uint32_t kIdentifyFinishOnRateMs      = 950;
constexpr uint32_t kIdentifyFinishOffRateMs     = 50;
constexpr uint32_t kIdentifyChannelChangeRateMs = 1000;
constexpr uint32_t kIdentifyBreatheRateMs       = 1000;

#if CONFIG_DUAL_MODE_SWTICH
const struct device * flash_para_dev = USER_PARTITION_DEVICE;
const struct device * zb_para_dev    = ZB_NVS_PARTITION_DEVICE;
constexpr int kDnssTimeout           = 60000;
static k_timer sDnssTimer;
#endif

#if APP_SET_NETWORK_COMM_ENDPOINT_SEC
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;
#endif

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));

k_timer sFactoryResetTimer;
uint8_t sFactoryResetCntr = 0;

bool sIsCommissioningFailed = false;
bool sIsNetworkProvisioned  = false;
bool sIsNetworkEnabled      = false;
bool sIsNetworkAttached     = false;
bool sHaveBLEConnections    = false;

#if APP_SET_DEVICE_INFO_PROVIDER
chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
#endif

#ifndef IDENTIFY_CLUSTER_DISABLED

void OnIdentifyTriggerEffect(Identify * identify)
{
    AppTaskCommon::IdentifyEffectHandler(identify->mCurrentEffectIdentifier);
}

Identify sIdentify = {
    kExampleEndpointId,           AppTask::IdentifyStartHandler,
    AppTask::IdentifyStopHandler, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnIdentifyTriggerEffect,
};

#endif

/**
 * @brief Set deferred attributes storage
 *
 * @see Define a custom attribute persister which makes actual write of the CurrentHue, CurrentSaturation, CurrentLevel attributes
 * value to the non-volatile storage only when it has remained constant for 5 seconds. This is to reduce the flash wearout when the
 * attribute changes frequently as a result of MoveToLevel command. DeferredAttribute object describes a deferred attribute, but
 * also holds a buffer with a value to be written, so it must live so long as the DeferredAttributePersistenceProvider object.
 *
 * @param ATTRIBUTES_ARRAY_SIZE The lenght of the DeferredAttribute array
 * @param DEFERRED_STORAGE_TIME The deferred time(ms) to store attributes
 */
#define ATTRIBUTES_ARRAY_SIZE (3U)
#define DEFERRED_STORAGE_TIME (500U)

DeferredAttribute gPersisters[] = {
#if CONFIG_DEFERRED_ATTR_STORAGE
    DeferredAttribute(
        ConcreteAttributePath(kExampleEndpointId, Clusters::ColorControl::Id, Clusters::ColorControl::Attributes::CurrentHue::Id)),
    DeferredAttribute(ConcreteAttributePath(kExampleEndpointId, Clusters::ColorControl::Id,
                                            Clusters::ColorControl::Attributes::CurrentSaturation::Id)),
    DeferredAttribute(
        ConcreteAttributePath(kExampleEndpointId, Clusters::LevelControl::Id, Clusters::LevelControl::Attributes::CurrentLevel::Id))
#endif // CONFIG_DEFERRED_ATTR_STORAGE
};

// Deferred persistence will be auto-initialized as soon as the default persistence is initialized
DefaultAttributePersistenceProvider gSimpleAttributePersistence;
DeferredAttributePersistenceProvider gDeferredAttributePersister(gSimpleAttributePersistence,
                                                                 Span<DeferredAttribute>(gPersisters, ATTRIBUTES_ARRAY_SIZE),
                                                                 System::Clock::Milliseconds32(DEFERRED_STORAGE_TIME));

// NOTE! This key is for test/certification only and should not be available in production devices!
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

class AppCallbacks : public AppDelegate
{
    bool isComissioningStarted;

public:
    void OnCommissioningSessionEstablishmentStarted() override { sIsCommissioningFailed = false; }
    void OnCommissioningSessionStarted() override { isComissioningStarted = true; }
    void OnCommissioningSessionStopped() override { isComissioningStarted = false; }
    void OnCommissioningSessionEstablishmentError(CHIP_ERROR err) override { sIsCommissioningFailed = true; }
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    void OnCommissioningWindowClosed() override
    {
        if (!isComissioningStarted)
            chip::DeviceLayer::Internal::BLEMgr().Shutdown();
    }
#endif
};

AppCallbacks sCallbacks;
} // namespace

#if CONFIG_STARTUP_OPTIMIZATION
volatile bool uart_init_flag           = false;
const struct device * cluster_para_dev = USER_CLUSTER_PARTITION_DEVICE;

uint32_t cluster_para_addr = USER_CLUSTER_PARTITION_OFFSET;
#define CLUSTER_PARA_LEN (sizeof(cluster_startup_para))
#define USER_CLUSTER_PARTITION_END (USER_CLUSTER_PARTITION_OFFSET + USER_CLUSTER_PARTITION_SIZE)

void set_debug_flag(bool flag)
{
    uart_init_flag = flag;
}

volatile bool read_debug_flag(void)
{
    return uart_init_flag;
}

void clear_cluster_para(void)
{
    flash_erase(cluster_para_dev, USER_CLUSTER_PARTITION_OFFSET, USER_CLUSTER_PARTITION_SIZE);
    cluster_para_addr = USER_CLUSTER_PARTITION_OFFSET;
}

void init_cluster_partition(void)
{
    uint32_t i, cur_addr;
    cluster_startup_para t_cmp;
    cluster_startup_para t_cmp_back;
    memset((void *) (&t_cmp_back), 0xff, CLUSTER_PARA_LEN);

    for (i = 0;; i++)
    {
        cur_addr = USER_CLUSTER_PARTITION_OFFSET + i * CLUSTER_PARA_LEN;
        if (cur_addr >= USER_CLUSTER_PARTITION_END)
        {
            clear_cluster_para();
            if (uart_init_flag)
            {
                printk("The partition is full, clear and redirect to:0x%x\n", cluster_para_addr);
            }
            break;
        }

        flash_read(cluster_para_dev, cur_addr, &t_cmp, CLUSTER_PARA_LEN);
        if (memcmp(&t_cmp, &t_cmp_back, CLUSTER_PARA_LEN) == 0) // read t_cmp is 0xff
        {
            cluster_para_addr = cur_addr;
            if (uart_init_flag)
            {
                printk("Init partition to:0x%x\n", cluster_para_addr);
            }
            return;
        }
    }
}

int store_cluster_para(cluster_startup_para * data)
{
    if (data == NULL)
    {
        if (uart_init_flag)
        {
            printk("The data that needs to be stored is NULL\n");
        }
        return -1;
    }
    if (cluster_para_addr >= (USER_CLUSTER_PARTITION_END - CLUSTER_PARA_LEN))
    {
        clear_cluster_para();
    }

    flash_write(cluster_para_dev, cluster_para_addr, data, CLUSTER_PARA_LEN);
    cluster_para_addr += CLUSTER_PARA_LEN;
    return 0;
}

int read_cluster_para(cluster_startup_para * data)
{
    if (data == NULL)
    {
        if (uart_init_flag)
        {
            printk("The data to be read is NULL\n");
        }
        return -1;
    }
    if (cluster_para_addr >= USER_CLUSTER_PARTITION_END)
    {
        clear_cluster_para();
        if (uart_init_flag)
        {
            printk("The read address exceeds partition, clear and redirect to:0x%x\n", cluster_para_addr);
        }
        return -1;
    }
    if ((cluster_para_addr - CLUSTER_PARA_LEN) < USER_CLUSTER_PARTITION_OFFSET)
    {
        if (uart_init_flag)
        {
            printk("The partition is all NULL\n");
        }
        return -1;
    }

    cluster_startup_para t_cmp;
    cluster_startup_para t_cmp_back;
    memset((void *) (&t_cmp_back), 0xff, CLUSTER_PARA_LEN);

    flash_read(cluster_para_dev, (cluster_para_addr - CLUSTER_PARA_LEN), &t_cmp, CLUSTER_PARA_LEN);
    if (memcmp(&t_cmp, &t_cmp_back, CLUSTER_PARA_LEN) == 0) // read t_cmp is 0xff, error
    {
        clear_cluster_para();
        if (uart_init_flag)
        {
            printk("The previous data was not successfully written\n");
        }
        return -1;
    }
    memcpy(data, &t_cmp, CLUSTER_PARA_LEN);
    return 0;
}
#endif // CONFIG_STARTUP_OPTIMIZATION

#if CONFIG_DUAL_MODE_SWTICH
void FactoryResetExtHandler(void)
{
    // Erase the user parameters partition to reset mode settings
    flash_erase(flash_para_dev, USER_PARTITION_OFFSET, USER_PARTITION_SIZE);
    // Erase ZigBee NVS data during factory reset
    flash_erase(zb_para_dev, ZB_NVS_START_ADR, ZB_NVS_SEC_SIZE);
#if CONFIG_STARTUP_OPTIMIZATION
    // Need to erase cluster para parition
    flash_erase(cluster_para_dev, USER_CLUSTER_PARTITION_OFFSET, USER_CLUSTER_PARTITION_SIZE);
#endif // CONFIG_STARTUP_OPTIMIZATION
}
#endif

class AppFabricTableDelegate : public FabricTable::Delegate
{
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
    {
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            ChipLogProgress(DeviceLayer, "Performing erasing of settings partition");

            // Do FactoryReset in case of failed commissioning to allow new pairing via BLE
            if (sIsCommissioningFailed)
            {
                chip::Server::GetInstance().ScheduleFactoryReset();
            }
            // TC-OPCREDS-3.6 (device doesn't need to reboot automatically after the last fabric is removed) can't use FactoryReset
            else
            {
                void * storage = nullptr;
                int status     = settings_storage_get(&storage);

                if (!status)
                {
                    status = nvs_clear(static_cast<nvs_fs *>(storage));
                }

                if (!status)
                {
                    status = nvs_mount(static_cast<nvs_fs *>(storage));
                }

                if (status)
                {
                    ChipLogError(DeviceLayer, "Storage clearance failed: %d", status);
                }
#if CONFIG_DUAL_MODE_SWTICH
                printk("Erasing user parameters and resetting to Zigbee mode");
                FactoryResetExtHandler();
                chip::Server::GetInstance().ScheduleFactoryReset();
#endif
            }
        }
    }
};

class PlatformMgrDelegate : public DeviceLayer::PlatformManagerDelegate
{
    // Disable openthread before reset to prevent writing to NVS
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    void OnShutDown() override
    {
        if (ThreadStackManagerImpl().IsThreadEnabled())
        {
            ThreadStackManagerImpl().Finalize();
        }
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
};

#if CONFIG_CHIP_LIB_SHELL
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>

static int cmd_telink_reboot(const struct shell * shell, size_t argc, char ** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(shell, "Performing board reboot...");
    sys_reboot(0);

    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_telink, SHELL_CMD(reboot, NULL, "Reboot board command", cmd_telink_reboot),
                               SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(telink, &sub_telink, "Telink commands", NULL);
#endif // CONFIG_CHIP_LIB_SHELL

#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
void AppTaskCommon::PowerOnFactoryReset(void)
{
    LOG_INF("schedule factory reset");
    chip::Server::GetInstance().ScheduleFactoryReset();
}
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */

#if CONFIG_DUAL_MODE_SWTICH

void SwitchBackToZigbee()
{
    uint8_t switch_flag = USER_MATTER_BACK_ZB;
    flash_write(flash_para_dev, USER_PARTITION_OFFSET, &switch_flag, 1);
    sys_reboot(0);
}

void AppTaskCommon::DnssTimerTimeoutCallback(k_timer * timer)
{
    if (!timer)
    {
        return;
    }
    /*
     * If Dnss initialization takes longer than 60 seconds,
     * the device will reboot and revert to Zigbee mode.
     */
    if (sBoot_zb)
    {
        printk("Matter: DnssTimer expired. Rebooting...\n");
        SwitchBackToZigbee();
    }
}
#endif

CHIP_ERROR AppTaskCommon::StartApp(void)
{

#if CONFIG_DUAL_MODE_SWTICH
    /*
     * If the device boots from Zigbee, set a flag and adjust user parameters.
     * Then start a timer to ensure Dnss initialization completes within
     * a given timeframe.
     */
    flash_read(flash_para_dev, USER_PARTITION_OFFSET, &user_para, sizeof(user_para));
    /* If booting from Zigbee mode, set a flag and clean deprecated parameter sectors. */
    if (user_para.val == USER_ZB_SW_VAL)
    {
        sBoot_zb = 1;
        /* If switching from ZigBee mode, get all existing clusters information. */
        flash_read(flash_para_dev, USER_PARTITION_OFFSET + sizeof(user_para), &light_para, sizeof(light_para));
        /* Ensure brightness is at least 2 to avoid display issues on HomePod Mini */
        if (light_para.level < 2)
        {
            light_para.level = 2;
        }
        /*
         * Pass the brightness value to the initialization code
         * to avoid gaps in PWM pool initialization.
         */
        if (light_para.onoff)
        {
            para_lightness = light_para.level;
        }
        k_timer_init(&sDnssTimer, &AppTask::DnssTimerTimeoutCallback, nullptr);
        k_timer_start(&sDnssTimer, K_MSEC(kDnssTimeout), K_NO_WAIT);
        printk("Matter: Started DNS protection timer.");
    }
#endif

    CHIP_ERROR err = GetAppTask().Init();

    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("AppTask Init fail");
        return err;
    }

    AppEvent event = {};

#if !CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    StartThreadButtonEventHandler();
#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
    StartWiFiButtonEventHandler();
#endif
#endif /* CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE */

#ifdef CONFIG_BOOTLOADER_MCUBOOT
    if (!sIsNetworkProvisioned)
    {
        LOG_INF("Confirm image.");
        OtaConfirmNewImage();
    }
#endif /* CONFIG_BOOTLOADER_MCUBOOT */

    while (true)
    {
        GetEvent(&event);
        DispatchEvent(&event);
    }
}
void AppTaskCommon::PrintFirmwareInfo(void)
{
    LOG_INF("SW Version: %u, %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION, CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

#if CONFIG_CHIP_APP_LOG_LEVEL > 3
    LOG_DBG("Matter revision: ");
    LOG_DBG("\t board: %s", CONFIG_BOARD);
    LOG_DBG("\t branch: %s %.8s%s %s", MATTER_BRANCH, MATTER_COMMIT_HASH, MATTER_LOCAL_STATUS, MATTER_COMMIT_DATE);
    LOG_DBG("\t remote: %s", MATTER_REMOTE_URL);
    LOG_DBG("\t build timestamp: %s", BUILD_TIMESTAMP);

    LOG_DBG("Zephyr revision: ");
    LOG_DBG("\t branch: %s %.8s%s %s", ZEPHYR_BRANCH, ZEPHYR_COMMIT_HASH, ZEPHYR_LOCAL_STATUS, ZEPHYR_COMMIT_DATE);
    LOG_DBG("\t remote: %s", ZEPHYR_REMOTE_URL);
    LOG_DBG("\t HAL commit: %.8s%s %s", TELINK_HAL_COMMIT_HASH, TELINK_HAL_LOCAL_STATUS, TELINK_HAL_COMMIT_DATE);
#endif
}

/**
 * @brief Use analog register to store OTA status.
 *
 * @see There are many OTA callback events in
 * AppTaskCommon::OtaEventsHandler, please embed it as needed.
 * The available analog registers address are as follows:
 *  TLSR9528 - 0x3b
 *  TL3218X  - 0x3b
 */
#if CONFIG_STORAGE_OTA_STATUS
#define ANALOG_REG_ADR (0x3b)
#define ANALOG_OTA_FLAG_VAL (0x55)

void OtaSetAnaFlag(void)
{
    int err = analog_write(ANALOG_REG_ADR, ANALOG_OTA_FLAG_VAL);

    if (!err)
    {
        printk("Write successful\n");
    }
    else
    {
        printk("The address:0x%x written is incorrect\n", ANALOG_REG_ADR);
    }
}

bool OtaGetAnaFlag(void)
{
    if (analog_read(ANALOG_REG_ADR) == ANALOG_OTA_FLAG_VAL)
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif // CONFIG_STORAGE_OTA_STATUS

/**
 * @brief Control factory reset base on button.
 *
 * @see It must set CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE=y
 * in prj.conf if you need to use independent button to control
 * factory reset. Otherwise, the matrix key will be used by default.
 */
#if CONFIG_BUTTON_FACTORY_RESET
void AppTaskCommon::ButtonFactoryReset(void)
{
    ButtonManager & buttonManager = ButtonManager::getInstance();
    buttonManager.addCallback(AppTaskCommon::FactoryResetButtonEventHandler, 0, true);

#if CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
    buttonManager.linkBackend(ButtonPool::getInstance());
#else
    buttonManager.linkBackend(ButtonMatrix::getInstance());
#endif // CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
}
#endif // CONFIG_BUTTON_FACTORY_RESET

CHIP_ERROR AppTaskCommon::InitCommonParts(void)
{
    CHIP_ERROR err;

    PrintFirmwareInfo();

/* Leave empty condition clause for development on customers boards */
#if CONFIG_CUSTOMER_MODE

#else
    InitLeds();
    UpdateStatusLED();

    InitPwms();

    InitButtons();
#endif

    // Initialize function button timer
    k_timer_init(&sFactoryResetTimer, &AppTask::FactoryResetTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sFactoryResetTimer, this);

    // Initialize CHIP server
#if CONFIG_CHIP_FACTORY_DATA
    ReturnErrorOnFailure(mFactoryDataProvider.Init());
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);
    SetCommissionableDataProvider(&mFactoryDataProvider);
    // Read EnableKey from the factory data.
    MutableByteSpan enableKey(sTestEventTriggerEnableKey);
    err = mFactoryDataProvider.GetEnableKey(enableKey);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("mFactoryDataProvider.GetEnableKey() failed. Could not delegate a test event trigger");
        memset(sTestEventTriggerEnableKey, 0, sizeof(sTestEventTriggerEnableKey));
    }
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif

    // Init ZCL Data Model and start server
    static CommonCaseDeviceServerInitParams initParams;
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
    VerifyOrDie(sTestEventTriggerDelegate.Init(ByteSpan(sTestEventTriggerEnableKey)) == CHIP_NO_ERROR);
#if CONFIG_CHIP_OTA_REQUESTOR
    static OTATestEventTriggerHandler sOtaTestEventTriggerHandler{};
    VerifyOrDie(sTestEventTriggerDelegate.AddHandler(&sOtaTestEventTriggerHandler) == CHIP_NO_ERROR);
#endif
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    VerifyOrDie(gSimpleAttributePersistence.Init(initParams.persistentStorageDelegate) == CHIP_NO_ERROR);

    initParams.dataModelProvider        = CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    initParams.appDelegate              = &sCallbacks;
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;
    ReturnErrorOnFailure(chip::Server::GetInstance().Init(initParams));

#if APP_SET_DEVICE_INFO_PROVIDER
    gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);
#endif

    /* add deferred storage attribute for provider */
    app::SetAttributePersistenceProvider(&gDeferredAttributePersister);

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

#if APP_SET_NETWORK_COMM_ENDPOINT_SEC
    // We only have network commissioning on endpoint 0.
    // Set up a valid Network Commissioning cluster on endpoint 0 is done in
    // src/platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);
#endif

    // We need to disable OpenThread to prevent writing to the NVS storage when factory reset occurs
    // The OpenThread thread is running during factory reset. The nvs_clear function is called during
    // factory reset, which makes the NVS storage innaccessible, but the OpenThread knows nothing
    // about this and tries to store the parameters to NVS. Because of this the OpenThread need to be
    // shut down before NVS. This delegate fixes the issue "Failed to store setting , ret -13",
    // which means that the NVS is already disabled.
    // For this the OnShutdown function is used
    PlatformMgr().SetDelegate(new PlatformMgrDelegate);

    // Add CHIP event handler and start CHIP thread.
    // Note that all the initialization code should happen prior to this point to avoid data races
    // between the main and the CHIP threads.
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

    err = chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(new AppFabricTableDelegate);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("AppFabricTableDelegate fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

void AppTaskCommon::IdentifyStartHandler(Identify *)
{
    AppEvent event;

    event.Type    = AppEvent::kEventType_IdentifyStart;
    event.Handler = [](AppEvent * event) {
        ChipLogProgress(Zcl, "OnIdentifyStart");
        PwmManager::getInstance().setPwmBlink(PwmManager::EAppPwm_Indication, kIdentifyBlinkRateMs, kIdentifyBlinkRateMs);
    };
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::IdentifyStopHandler(Identify *)
{
    AppEvent event;

    event.Type    = AppEvent::kEventType_IdentifyStop;
    event.Handler = [](AppEvent * event) {
        ChipLogProgress(Zcl, "OnIdentifyStop");
        PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Indication, false);
    };
    GetAppTask().PostEvent(&event);
}

#ifdef CONFIG_CHIP_PW_RPC
void AppTaskCommon::ButtonEventHandler(ButtonId_t btnId, bool btnPressed)
{
    if (!btnPressed)
    {
        return;
    }

    switch (btnId)
    {
    case kButtonId_ExampleAction:
        ExampleActionButtonEventHandler();
        break;
    case kButtonId_FactoryReset:
        FactoryResetButtonEventHandler();
        break;
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    case kButtonId_StartThread:
        StartThreadButtonEventHandler();
        break;
#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
    case kButtonId_StartWiFi:
        StartWiFiButtonEventHandler();
        break;
#endif
    case kButtonId_StartBleAdv:
        StartBleAdvButtonEventHandler();
        break;
    }
}
#endif

void AppTaskCommon::InitLeds()
{
    LedManager & ledManager = LedManager::getInstance();

    LinkLeds(ledManager);

    ledManager.linkBackend(LedPool::getInstance());
}

void AppTaskCommon::LinkLeds(LedManager & ledManager)
{
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    ledManager.linkLed(LedManager::EAppLed_Status, 0);
#endif // CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
}

void AppTaskCommon::InitPwms()
{
    PwmManager & pwmManager = PwmManager::getInstance();

    LinkPwms(pwmManager);

#if CONFIG_WS2812_STRIP_GPIO_TELINK
    pwmManager.linkBackend(Ws2812Strip::getInstance());
#elif CONFIG_PWM
    pwmManager.linkBackend(PwmPool::getInstance());
#else
    pwmManager.linkBackend(PwmDummy::getInstance());
#endif
}

void AppTaskCommon::LinkPwms(PwmManager & pwmManager)
{
#if CONFIG_BOARD_TLSR9118BDK40D_V1 && CONFIG_PWM // TLSR9118BDK40D_V1 EVK supports single LED PWM channel
    pwmManager.linkPwm(PwmManager::EAppPwm_Red, 0);
#elif CONFIG_WS2812_STRIP_GPIO_TELINK
    pwmManager.linkPwm(PwmManager::EAppPwm_Red, 0);
    pwmManager.linkPwm(PwmManager::EAppPwm_Green, 1);
    pwmManager.linkPwm(PwmManager::EAppPwm_Blue, 2);
#elif CONFIG_PWM
    pwmManager.linkPwm(PwmManager::EAppPwm_Indication, 0);
    pwmManager.linkPwm(PwmManager::EAppPwm_Red, 1);
    pwmManager.linkPwm(PwmManager::EAppPwm_Green, 2);
    pwmManager.linkPwm(PwmManager::EAppPwm_Blue, 3);
#endif
}

void AppTaskCommon::InitButtons(void)
{
    ButtonManager & buttonManager = ButtonManager::getInstance();

    LinkButtons(buttonManager);

#if CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
    buttonManager.linkBackend(ButtonPool::getInstance());
#else
    buttonManager.linkBackend(ButtonMatrix::getInstance());
#endif // CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
}

void AppTaskCommon::LinkButtons(ButtonManager & buttonManager)
{
    buttonManager.addCallback(FactoryResetButtonEventHandler, 0, true);
    buttonManager.addCallback(ExampleActionButtonEventHandler, 1, true);
#if CONFIG_TELINK_OTA_BUTTON_TEST
    buttonManager.addCallback(TestOTAButtonEventHandler, 2, true);
#else
    buttonManager.addCallback(StartBleAdvButtonEventHandler, 2, true);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    buttonManager.addCallback(StartThreadButtonEventHandler, 3, true);
#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
    buttonManager.addCallback(StartWiFiButtonEventHandler, 3, true);
#endif
}

void AppTaskCommon::UpdateStatusLED()
{
    if (sIsNetworkProvisioned && sIsNetworkEnabled)
    {
        if (sIsNetworkAttached)
        {
            LedManager::getInstance().setLed(LedManager::EAppLed_Status, 950, 50);
        }
        else
        {
            LedManager::getInstance().setLed(LedManager::EAppLed_Status, 100, 100);
        }
    }
    else
    {
        LedManager::getInstance().setLed(LedManager::EAppLed_Status, 50, 950);
    }
}

void AppTaskCommon::IdentifyEffectHandler(Clusters::Identify::EffectIdentifierEnum aEffect)
{
    switch (aEffect)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink");
        PwmManager::getInstance().setPwmBlink(PwmManager::EAppPwm_Indication, kIdentifyBlinkRateMs, kIdentifyBlinkRateMs);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe");
        PwmManager::getInstance().setPwmBreath(PwmManager::EAppPwm_Indication, kIdentifyBreatheRateMs);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay");
        PwmManager::getInstance().setPwmBlink(PwmManager::EAppPwm_Indication, kIdentifyOkayOnRateMs, kIdentifyOkayOffRateMs);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange");
        PwmManager::getInstance().setPwmBlink(PwmManager::EAppPwm_Indication, kIdentifyChannelChangeRateMs,
                                              kIdentifyChannelChangeRateMs);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kFinishEffect");
        PwmManager::getInstance().setPwmBlink(PwmManager::EAppPwm_Indication, kIdentifyFinishOnRateMs, kIdentifyFinishOffRateMs);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kStopEffect");
        PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Indication, false);
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }
}

void AppTaskCommon::StartBleAdvButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = StartBleAdvHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::StartBleAdvHandler(AppEvent * aEvent)
{
    LOG_INF("StartBleAdvHandler");

    // Disable manual Matter service BLE advertising after device provisioning.
    if (sIsNetworkProvisioned)
    {
        LOG_INF("Device already commissioned");
        return;
    }

    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        LOG_INF("BLE adv already enabled");
        return;
    }

    if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        LOG_ERR("OpenBasicCommissioningWindow fail");
    }
}

void AppTaskCommon::FactoryResetButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = FactoryResetHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::FactoryResetHandler(AppEvent * aEvent)
{
    if (sFactoryResetCntr == 0)
    {
        k_timer_start(&sFactoryResetTimer, K_MSEC(kFactoryResetCalcTimeout), K_NO_WAIT);
    }

    sFactoryResetCntr++;
    LOG_INF("Factory Reset Trigger Counter: %d/%d", sFactoryResetCntr, kFactoryResetTriggerCntr);

    if (sFactoryResetCntr == kFactoryResetTriggerCntr)
    {
        k_timer_stop(&sFactoryResetTimer);
        sFactoryResetCntr = 0;

#if CONFIG_DUAL_MODE_SWTICH
        printk("Factory reset triggered by button; reverting to Zigbee mode\n");
        FactoryResetExtHandler();
#endif
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

void AppTaskCommon::FactoryResetTimerTimeoutCallback(k_timer * timer)
{
    if (!timer)
    {
        return;
    }

    AppEvent event;
    event.Type    = AppEvent::kEventType_Timer;
    event.Handler = FactoryResetTimerEventHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::FactoryResetTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    sFactoryResetCntr = 0;
    LOG_INF("Factory Reset Trigger Counter is cleared");
}

#if CONFIG_TELINK_OTA_BUTTON_TEST
void AppTaskCommon::TestOTAButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = TestOTAHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::TestOTAHandler(AppEvent * aEvent)
{
    LOG_INF("TestOTAHandler");

    chip::DeviceLayer::OTAImageProcessorImpl imageProcessor;
    imageProcessor.Apply();
}
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
void AppTaskCommon::StartThreadButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = StartThreadHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::StartThreadHandler(AppEvent * aEvent)
{
    LOG_INF("StartThreadHandler");
    if (!sIsNetworkProvisioned)
    {
        // Switch context from BLE to Thread
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
        Internal::BLEManagerImpl sInstance;
        sInstance.SwitchToIeee802154();
#else
        ThreadStackMgrImpl().SetRadioBlocked(false);
        ThreadStackMgrImpl().SetThreadEnabled(true);
#endif
        StartDefaultThreadNetwork();
    }
    else
    {
        LOG_INF("Device already commissioned");
    }
}

#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
void AppTaskCommon::StartWiFiButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = StartWiFiHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::StartWiFiHandler(AppEvent * aEvent)
{
    LOG_INF("StartWiFiHandler");

    if (!strlen(CONFIG_DEFAULT_WIFI_SSID) || !strlen(CONFIG_DEFAULT_WIFI_PASSWORD))
    {
        LOG_ERR("default WiFi SSID/Password are not set");
    }

    if (!sIsNetworkProvisioned)
    {
        net_if_up(InetUtils::GetWiFiInterface());
        NetworkCommissioning::TelinkWiFiDriver().StartDefaultWiFiNetwork();
    }
    else
    {
        LOG_INF("Device already commissioned");
    }
}
#endif

void AppTaskCommon::ExampleActionButtonEventHandler(void)
{
    AppEvent event;

    if (!GetAppTask().ExampleActionEventHandler)
    {
        return;
    }

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = GetAppTask().ExampleActionEventHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::SetExampleButtonCallbacks(EventHandler aAction_CB)
{
    ExampleActionEventHandler = aAction_CB;
}

void AppTaskCommon::OtaEventsHandler(const ChipDeviceEvent * event)
{
    switch (event->OtaStateChanged.newState)
    {
    case DeviceLayer::kOtaDownloadInProgress:
        ChipLogProgress(DeviceLayer, "OTA image download in progress\n");
        break;
    case DeviceLayer::kOtaDownloadComplete:
        ChipLogProgress(DeviceLayer, "OTA image download complete\n");
        break;
    case DeviceLayer::kOtaDownloadFailed:
        ChipLogProgress(DeviceLayer, "OTA image download failed\n");
        break;
    case DeviceLayer::kOtaDownloadAborted:
        ChipLogProgress(DeviceLayer, "OTA image download aborted\n");
        break;
    case DeviceLayer::kOtaApplyInProgress:
        ChipLogProgress(DeviceLayer, "OTA image apply in progress\n");
        break;
    case DeviceLayer::kOtaApplyComplete:
        ChipLogProgress(DeviceLayer, "OTA image apply complete\n");
        break;
    case DeviceLayer::kOtaApplyFailed:
        ChipLogProgress(DeviceLayer, "OTA image apply failed\n");
        break;
    default:
        break;
    }
}

void AppTaskCommon::ChipEventHandler(const ChipDeviceEvent * event, intptr_t /* arg */)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        sHaveBLEConnections = ConnectivityMgr().NumBLEConnections() != 0;
        UpdateStatusLED();
#ifdef CONFIG_CHIP_NFC_ONBOARDING_PAYLOAD
        if (event->CHIPoBLEAdvertisingChange.Result == kActivity_Started)
        {
            if (NFCOnboardingPayloadMgr().IsTagEmulationStarted())
            {
                LOG_INF("NFC Tag emulation is already started");
            }
            else
            {
                ShareQRCodeOverNFC(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
            }
        }
        else if (event->CHIPoBLEAdvertisingChange.Result == kActivity_Stopped)
        {
            NFCOnboardingPayloadMgr().StopTagEmulation();
        }
#endif
        break;
#if CONFIG_DUAL_MODE_SWTICH
    case DeviceEventType::kCommissioningComplete: {
        uint8_t val = USER_MATTER_PAIR_VAL;
#if CONFIG_CUSTOMER_MODE
        /* need to add here to update the cluster information , only in the zb switch and touchlink is paired*/
        if (user_para.val == USER_ZB_SW_VAL && user_para.on_net)
        {
            Protocols::InteractionModel::Status status;
            /* Update all cluster information, only if switching from ZigBee mode and the touchlink is paired. */
            status = Clusters::OnOff::Attributes::OnOff::Set(kExampleEndpointId, light_para.onoff);
            if (status != Protocols::InteractionModel::Status::Success)
            {
                LOG_ERR("Update OnOff fail: %x", to_underlying(status));
            }
            status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kExampleEndpointId, light_para.level);
            {
                LOG_ERR("Update brightness fail: %x", to_underlying(status));
            }
        }
#endif
        sBoot_zb = 0;
        flash_erase(flash_para_dev, USER_PARTITION_OFFSET, USER_PARTITION_SIZE);
        flash_write(flash_para_dev, USER_PARTITION_OFFSET, &val, 1);
#if CONFIG_STARTUP_OPTIMIZATION
        cluster_startup_para cluster_para;
        cluster_para.onoff = light_para.onoff;
        cluster_para.level = light_para.level;
        if (store_cluster_para(&cluster_para) != 0)
        {
            if (uart_init_flag)
            {
                printk("Fail store cluster parameter after commissioning complete\n");
            }
        }
#endif // CONFIG_STARTUP_OPTIMIZATION
        printk("Commissioning complete; Matter commissioned flag set.\n");
    }
    break;
    case DeviceEventType::kFailSafeTimerExpired:
        /* Reset to Zigbee mode if commissioning fails */
        if (sBoot_zb)
        {
            printk("FailSafeTimer expired; Matter commissioning failed. Rebooting to Zigbee mode...\n");
            SwitchBackToZigbee();
        }
        else
        {
            printk("FailSafeTimer expired; Matter commissioning failed.\n");
        }
        break;
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    case DeviceEventType::kDnssdInitialized:
#if CONFIG_CHIP_OTA_REQUESTOR
        InitBasicOTARequestor();
        if (GetRequestorInstance()->GetCurrentUpdateState() == Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kIdle)
        {
#endif
#ifdef CONFIG_BOOTLOADER_MCUBOOT
            OtaConfirmNewImage();
#endif /* CONFIG_BOOTLOADER_MCUBOOT */
#if CONFIG_CHIP_OTA_REQUESTOR
        }
#endif

#if CONFIG_DUAL_MODE_SWTICH
        if (sBoot_zb)
        {
            k_timer_stop(&sDnssTimer);
            printk("DnssTimer stopped; DNS-SD has been initialized.\n");
        }
#endif
        break;
    case DeviceEventType::kThreadStateChange:
        sIsNetworkProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsNetworkEnabled     = ConnectivityMgr().IsThreadEnabled();
        sIsNetworkAttached    = ConnectivityMgr().IsThreadAttached();
#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
    case DeviceEventType::kWiFiConnectivityChange:
        sIsNetworkProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
        sIsNetworkEnabled     = ConnectivityMgr().IsWiFiStationEnabled();
        sIsNetworkAttached    = ConnectivityMgr().IsWiFiStationConnected();
#if CONFIG_CHIP_OTA_REQUESTOR
        if (event->WiFiConnectivityChange.Result == kConnectivity_Established)
        {
            InitBasicOTARequestor();
        }
#endif
#endif /* CHIP_DEVICE_CONFIG_ENABLE_THREAD */
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        UpdateStatusLED();
#endif
        break;
    case DeviceEventType::kOtaStateChanged:
        AppTaskCommon::OtaEventsHandler(event);
        break;
    default:
        break;
    }
}

void AppTaskCommon::PostEvent(AppEvent * aEvent)
{
    if (!aEvent)
        return;
    if (k_msgq_put(&sAppEventQueue, aEvent, K_NO_WAIT) != 0)
    {
        LOG_INF("PostEvent fail");
    }
}

void AppTaskCommon::DispatchEvent(AppEvent * aEvent)
{
    if (!aEvent)
        return;
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        LOG_INF("Dropping event without handler");
    }
}

void AppTaskCommon::GetEvent(AppEvent * aEvent)
{
    k_msgq_get(&sAppEventQueue, aEvent, K_FOREVER);
}
