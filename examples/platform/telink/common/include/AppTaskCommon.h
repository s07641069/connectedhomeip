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

#pragma once

#include "AppConfig.h"
#include "AppEventCommon.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <platform/CHIPDeviceLayer.h>

#if CONFIG_CHIP_FACTORY_DATA
#include <platform/telink/FactoryDataProvider.h>
#endif

#ifdef CONFIG_CHIP_PW_RPC
#include "Rpc.h"
#endif

#include <credentials/examples/DeviceAttestationCredsExample.h>

#if CONFIG_DUAL_MODE_SWTICH
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/reboot.h>

#if CONFIG_SOC_RISCV_TELINK_B92
#define ZB_NVS_PARTITION zigbee_partition
#define ZB_NVS_SEC_SIZE FIXED_PARTITION_SIZE(ZB_NVS_PARTITION)
#elif CONFIG_SOC_RISCV_TELINK_TL321X
#define ZB_NVS_PARTITION slot1_partition
/* zb para locate in the slot1 , and it will cost 104k size in slot1 */
#define ZB_NVS_SEC_SIZE (104 * 1024)
#endif

#define ZB_NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(ZB_NVS_PARTITION)
#define ZB_NVS_START_ADR FIXED_PARTITION_OFFSET(ZB_NVS_PARTITION)

#define USER_INIT_VAL 0xff
#define USER_ZB_SW_VAL 0xaa
#define USER_MATTER_PAIR_VAL 0x55
#define USER_MATTER_BACK_ZB 0xa0 // only commisiion fail will back to zb
#define USER_PARA_MAC_OFFSET 0x100
#define USER_PARTITION user_para_partition
#define USER_PARTITION_DEVICE FIXED_PARTITION_DEVICE(USER_PARTITION)
#define USER_PARTITION_OFFSET FIXED_PARTITION_OFFSET(USER_PARTITION)
#define USER_PARTITION_SIZE FIXED_PARTITION_SIZE(USER_PARTITION)

typedef struct
{
    uint8_t val;
    uint8_t on_net;
} user_para_t;

typedef struct
{
    uint8_t onoff;
    uint8_t level;
    uint16_t color_temp_mireds;
    uint16_t currentx;
    uint16_t currenty;
    uint16_t enhanced_current_hue;
    uint16_t onoff_transition;
    uint8_t cur_hue;
    uint8_t cur_saturation;
    uint8_t color_mode;
} light_para_t;

extern light_para_t light_para;
extern user_para_t user_para;
extern uint8_t para_lightness;
extern uint8_t sBoot_zb;
#endif

#if CONFIG_STARTUP_OPTIMIZATION
#define USER_CLUSTER_PARTITION user_cluster_partition
#define USER_CLUSTER_PARTITION_DEVICE FIXED_PARTITION_DEVICE(USER_CLUSTER_PARTITION)
#define USER_CLUSTER_PARTITION_OFFSET FIXED_PARTITION_OFFSET(USER_CLUSTER_PARTITION)
#define USER_CLUSTER_PARTITION_SIZE FIXED_PARTITION_SIZE(USER_CLUSTER_PARTITION)

typedef struct
{
    uint8_t onoff;
    uint8_t level;
    uint8_t rfu[30];
} cluster_startup_para;

extern volatile bool uart_init_flag;

void set_debug_flag(bool flag);
volatile bool read_debug_flag(void);
void init_cluster_partition(void);
int store_cluster_para(cluster_startup_para * data);
int read_cluster_para(cluster_startup_para * data);
#endif // CONFIG_STARTUP_OPTIMIZATION

#include <cstdint>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace {
inline constexpr EndpointId kExampleEndpointId = 1;
inline constexpr uint8_t kDefaultMinLevel      = 0;
inline constexpr uint8_t kDefaultMaxLevel      = 254;
inline constexpr uint8_t kButtonPushEvent      = 1;
inline constexpr uint8_t kButtonReleaseEvent   = 0;
} // namespace

class LedManager;
class PwmManager;
class ButtonManager;

struct Identify;

class AppTaskCommon
{
public:
#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
    void PowerOnFactoryReset(void);
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */
    CHIP_ERROR StartApp();
    void PostEvent(AppEvent * event);

    static void IdentifyEffectHandler(Clusters::Identify::EffectIdentifierEnum aEffect);
    static void IdentifyStartHandler(Identify *);
    static void IdentifyStopHandler(Identify *);

#ifdef CONFIG_CHIP_PW_RPC
    enum ButtonId_t
    {
        kButtonId_ExampleAction = 1,
        kButtonId_FactoryReset,
        kButtonId_StartWiFi,
        kButtonId_StartThread,
        kButtonId_StartBleAdv
    } ButtonId;
#endif

protected:
    CHIP_ERROR InitCommonParts(void);
    void PrintFirmwareInfo(void);

    void DispatchEvent(AppEvent * event);
    void GetEvent(AppEvent * aEvent);

    void InitLeds();
    virtual void LinkLeds(LedManager & ledManager);
    void InitPwms();
    virtual void LinkPwms(PwmManager & pwmManager);
    void InitButtons(void);
    virtual void LinkButtons(ButtonManager & buttonManager);

    static void FactoryResetTimerTimeoutCallback(k_timer * timer);
    static void DnssTimerTimeoutCallback(k_timer * timer);
    static void FactoryResetTimerEventHandler(AppEvent * aEvent);
    static void FactoryResetButtonEventHandler(void);
    static void FactoryResetHandler(AppEvent * aEvent);

    static void StartBleAdvButtonEventHandler(void);
    static void StartBleAdvHandler(AppEvent * aEvent);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    static void StartThreadButtonEventHandler(void);
    static void StartThreadHandler(AppEvent * aEvent);
#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
    static void StartWiFiButtonEventHandler(void);
    static void StartWiFiHandler(AppEvent * aEvent);
#endif

#if CONFIG_TELINK_OTA_BUTTON_TEST
    static void TestOTAButtonEventHandler(void);
    static void TestOTAHandler(AppEvent * aEvent);
#endif

#if CONFIG_BUTTON_FACTORY_RESET
    static void ButtonFactoryReset(void);
#endif // CONFIG_BUTTON_FACTORY_RESET

    static void ExampleActionButtonEventHandler(void);

    void SetExampleButtonCallbacks(EventHandler aAction_CB);
    EventHandler ExampleActionEventHandler;

    static void OtaEventsHandler(const chip::DeviceLayer::ChipDeviceEvent * event);
    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    static void UpdateStatusLED(void);

#if CONFIG_CHIP_FACTORY_DATA
    chip::DeviceLayer::FactoryDataProvider<chip::DeviceLayer::ExternalFlashFactoryData> mFactoryDataProvider;
#endif

#ifdef CONFIG_CHIP_PW_RPC
    friend class chip::rpc::TelinkButton;
    static void ButtonEventHandler(ButtonId_t btnId, bool btnPressed);
#endif
};
