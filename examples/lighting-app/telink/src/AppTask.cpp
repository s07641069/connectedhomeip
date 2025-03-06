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

#include "AppTask.h"
#include <app/server/Server.h>

#include "ColorFormat.h"
#include "LEDManager.h"
#include "PWMManager.h"
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

#include <app-common/zap-generated/attributes/Accessors.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace {
bool sfixture_on;
uint8_t sBrightness;
AppTask::Fixture_Action sColorAction = AppTask::INVALID_ACTION;
XyColor_t sXY;
HsvColor_t sHSV;
CtColor_t sCT;
RgbColor_t sLedRgb;
} // namespace

AppTask AppTask::sAppTask;

bool AppTask::IsTurnedOn() const
{
    return sfixture_on;
}

#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
void AppTask::PowerOnFactoryReset(void)
{
    LOG_INF("Lighting App Power On Factory Reset");
    AppEvent event;
    event.Type    = AppEvent::kEventType_DeviceAction;
    event.Handler = PowerOnFactoryResetEventHandler;
    GetAppTask().PostEvent(&event);
}
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */

#if CONFIG_CUSTOMER_MODE
/**
 * @brief Use I2C to transfer data.
 *
 * @see CONFIG_I2C=y must be set in prj.conf
 * when you need to use I2C module.
 */
void i2c_demo_proc(void)
{
#if CONFIG_I2C
    const uint8_t tx_buf[23] = { 0xc0, 0x63, 0x3f, 0x63, 0x63, 0x63, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00,
                                 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x2b, 0x06, 0xbe };
    /* add the i2c module here */
    printk("i2c demo start \n.");
    uint32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_FAST) | I2C_MODE_CONTROLLER;
    /* get i2c device */
    int rc;
    const struct i2c_dt_spec i2c = I2C_DT_SPEC_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(ledcontrol_i2c));
    if (!device_is_ready(i2c.bus))
    {
        printf("Device %s is not ready\n", i2c.bus->name);
        return;
    }
    rc = i2c_configure(i2c.bus, i2c_cfg);
    if (rc != 0)
    {
        printf("Failed to configure i2c device\n");
        return;
    }
    i2c_write(i2c.bus, tx_buf + 1, sizeof(tx_buf) - 1, tx_buf[0]);
    printk("i2c demo stop ,finish transfer\n");
#endif // CONFIG_I2C
}

/**
 * @brief Use ADC to sampling.
 *
 * @see CONFIG_ADC=y must be set in prj.conf
 * when you need to use ADC module.
 */

/* Data of ADC io-channels specified in devicetree. */
#define DT_SPEC_AND_COMMA(node_id, prop, idx) ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

void adc_demo_proc(void)
{
#if CONFIG_ADC
    static const struct adc_dt_spec adc_channels[] = { DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA) };

    /* Define ADC data structure. */
    uint16_t buf;
    struct adc_sequence sequence = {
        .buffer = &buf,
        /* buffer size in bytes, not number of samples */
        .buffer_size = sizeof(buf),
    };

    int err = 0;

    /* Configure channels individually prior to sampling. */
    for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
    {
        if (!device_is_ready(adc_channels[i].dev))
        {
            printf("ADC controller device %s not ready\n", adc_channels[i].dev->name);
            return;
        }
        err = adc_channel_setup_dt(&adc_channels[i]);
        if (err < 0)
        {
            printf("Could not setup channel #%d (%d)\n", i, err);
            return;
        }
    }

    /* ADC sampling. */
    for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
    {
        printf("- %s, channel %d: ", adc_channels[i].dev->name, adc_channels[i].channel_id);
        (void) adc_sequence_init_dt(&adc_channels[i], &sequence);
        err = adc_read(adc_channels[i].dev, &sequence);
        if (err < 0)
        {
            printf("Could not read (%d)\n", err);
            continue;
        }

        int32_t val_mv = 0;

        /*
         * If using differential mode, the 16 bit value
         * in the ADC sample buffer should be a signed 2's
         * complement value.
         */
        if (adc_channels[i].channel_cfg.differential)
        {
            val_mv = (int32_t) ((int16_t) buf);
        }
        else
        {
            val_mv = (int32_t) buf;
        }
        printf("%" PRId32, val_mv);

        /* conversion to mV may not be supported, skip if not */
        err = adc_raw_to_millivolts_dt(&adc_channels[i], &val_mv);
        if (err < 0)
        {
            printf(" (value in mV not available)\n");
        }
        else
        {
            printf(" = %" PRId32 " mV\n", val_mv);
        }
    }
#endif // CONFIG_ADC
}

/**
 * @brief Set cluster info.
 */
#define MATTER_COLORMODE_ENABLE 0

void AppTask::Set_cluster_info(void)
{
    printk("%%%%%%Set_cluster_info!!!!%%%%%%\n");
    light_para_t * p_para = &light_para;
    Protocols::InteractionModel::Status status;
    printk("%%%%%%Set_cluster_info:p_para->onoff:%d!!!!%%%%%%\n", p_para->onoff);
    status = Clusters::OnOff::Attributes::OnOff::Set(1, p_para->onoff);
    // Set brightness value
    printk("%%%%%%Set_cluster_info:p_para->level:%d!!!!%%%%%%\n", p_para->level);
    status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kExampleEndpointId, p_para->level);

#if MATTER_COLORMODE_ENABLE
    // Set ColorMode value
    printk("%%%%%%Set_cluster_info:p_para->color_mode:%d!!!!%%%%%%\n", p_para->color_mode);
    status = Clusters::ColorControl::Attributes::ColorMode::Set(1, p_para->color_mode);
#endif

    // Set ColorTemperatureMireds value
    status = Clusters::ColorControl::Attributes::ColorTemperatureMireds::Set(1, p_para->color_temp_mireds);
    // Set CurrentX value
    status = Clusters::ColorControl::Attributes::CurrentX::Set(1, p_para->currentx);
    // Set CurrentY value
    status = Clusters::ColorControl::Attributes::CurrentY::Set(1, p_para->currenty);
    // Set EnhancedCurrentHue value
    status = Clusters::ColorControl::Attributes::EnhancedCurrentHue::Set(1, p_para->enhanced_current_hue);
    // Set CurrentHue value
    status = Clusters::ColorControl::Attributes::CurrentHue::Set(1, p_para->cur_hue);
    // Set CurrentSaturation value
    status = Clusters::ColorControl::Attributes::CurrentSaturation::Set(1, p_para->cur_saturation);
    // Set OnOffTransitionTime value
    status = Clusters::LevelControl::Attributes::OnOffTransitionTime::Set(1, p_para->onoff_transition);
}

/**
 * @brief Get cluster info.
 */
void AppTask::Init_cluster_info(void)
{
    light_para_t * p_para = &light_para;
    Protocols::InteractionModel::Status status;
    bool onoff_sts;
    status        = Clusters::OnOff::Attributes::OnOff::Get(1, &(onoff_sts));
    p_para->onoff = (uint8_t) onoff_sts;
    app::DataModel::Nullable<uint8_t> brightness;
    // Read brightness value
    status = Clusters::LevelControl::Attributes::CurrentLevel::Get(kExampleEndpointId, brightness);
    if (status == Protocols::InteractionModel::Status::Success && !brightness.IsNull())
    {
        p_para->level = brightness.Value();
    }

#if MATTER_COLORMODE_ENABLE
    // Read ColorMode value
    status = Clusters::ColorControl::Attributes::ColorMode::Get(1, &(p_para->color_mode));
#endif

    // Read ColorTemperatureMireds value
    status = Clusters::ColorControl::Attributes::ColorTemperatureMireds::Get(1, &(p_para->color_temp_mireds));

    // Read CurrentX value
    status = Clusters::ColorControl::Attributes::CurrentX::Get(1, &(p_para->currentx));

    // Read CurrentY value
    status = Clusters::ColorControl::Attributes::CurrentY::Get(1, &(p_para->currenty));

    // Read EnhancedCurrentHue value
    status = Clusters::ColorControl::Attributes::EnhancedCurrentHue::Get(1, &(p_para->enhanced_current_hue));

    // Read CurrentHue value
    status = Clusters::ColorControl::Attributes::CurrentHue::Get(1, &(p_para->cur_hue));

    // Read CurrentSaturation value
    status = Clusters::ColorControl::Attributes::CurrentSaturation::Get(1, &(p_para->cur_saturation));

    // Read OnOffTransitionTime value
    status = Clusters::LevelControl::Attributes::OnOffTransitionTime::Get(1, &(p_para->onoff_transition));
}
#endif // CONFIG_CUSTOMER_MODE

CHIP_ERROR AppTask::Init(void)
{
    SetExampleButtonCallbacks(LightingActionEventHandler);
    InitCommonParts();

#if CONFIG_CUSTOMER_MODE
    if (user_para.val == USER_ZB_SW_VAL)
    {
        /* Switching from TouchLink (Zigbee) to Matter. Restore previous states. */
        Set_cluster_info();
    }
    else if (user_para.val == USER_MATTER_PAIR_VAL)
    {
        /* start from matter , add cluster info demo here*/
        Init_cluster_info();
    }
    else
    {
        /* start from matter , but without zigbee fw*/
    }
    /* if need , can call i2c_demo_proc here to see light works or not*/

#else
    Protocols::InteractionModel::Status status;

    app::DataModel::Nullable<uint8_t> brightness;
    // Read brightness value
    status = Clusters::LevelControl::Attributes::CurrentLevel::Get(kExampleEndpointId, brightness);
    if (status == Protocols::InteractionModel::Status::Success && !brightness.IsNull())
    {
        sBrightness = brightness.Value();
    }

    memset(&sLedRgb, sBrightness, sizeof(RgbColor_t));

    bool storedValue;
    // Read storedValue on/off value
    status = Clusters::OnOff::Attributes::OnOff::Get(1, &storedValue);
    if (status == Protocols::InteractionModel::Status::Success)
    {
        // Set actual state to stored before reboot
        SetInitiateAction(storedValue ? ON_ACTION : OFF_ACTION, static_cast<int32_t>(AppEvent::kEventType_DeviceAction), nullptr);
    }
#endif
    return CHIP_NO_ERROR;
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    Fixture_Action action = INVALID_ACTION;
    int32_t actor         = 0;

    if (aEvent->Type == AppEvent::kEventType_DeviceAction)
    {
        action = static_cast<Fixture_Action>(aEvent->DeviceEvent.Action);
        actor  = aEvent->DeviceEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        sfixture_on = !sfixture_on;

        sAppTask.UpdateClusterState();
    }
}

void AppTask::UpdateClusterState(void)
{
    Protocols::InteractionModel::Status status;
    bool isTurnedOn  = sfixture_on;
    uint8_t setLevel = sBrightness;

    // write the new on/off value
    status = Clusters::OnOff::Attributes::OnOff::Set(kExampleEndpointId, isTurnedOn);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        LOG_ERR("Update OnOff fail: %x", to_underlying(status));
    }

    status = Clusters::LevelControl::Attributes::CurrentLevel::Set(kExampleEndpointId, setLevel);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        LOG_ERR("Update CurrentLevel fail: %x", to_underlying(status));
    }
}

void AppTask::SetInitiateAction(Fixture_Action aAction, int32_t aActor, uint8_t * value)
{
    bool setRgbAction = false;

    if (aAction == ON_ACTION || aAction == OFF_ACTION)
    {
        if (aAction == ON_ACTION)
        {
            sfixture_on = true;
#ifdef CONFIG_PWM
            PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Red, (((uint32_t) sLedRgb.r * 1000) / UINT8_MAX));
            PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Green, (((uint32_t) sLedRgb.g * 1000) / UINT8_MAX));
            PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Blue, (((uint32_t) sLedRgb.b * 1000) / UINT8_MAX));
#else
            LedManager::getInstance().setLed(LedManager::EAppLed_App0, true);
#endif
        }
        else
        {
            sfixture_on = false;
#ifdef CONFIG_PWM
            PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Red, false);
            PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Green, false);
            PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Blue, false);
#else
            LedManager::getInstance().setLed(LedManager::EAppLed_App0, false);
#endif
        }
    }
    else if (aAction == LEVEL_ACTION)
    {
        // Save a new brightness for ColorControl
        sBrightness = *value;

        if (sColorAction == COLOR_ACTION_XY)
        {
            sLedRgb = XYToRgb(sBrightness, sXY.x, sXY.y);
        }
        else if (sColorAction == COLOR_ACTION_HSV)
        {
            sHSV.v  = sBrightness;
            sLedRgb = HsvToRgb(sHSV);
        }
        else
        {
            memset(&sLedRgb, sBrightness, sizeof(RgbColor_t));
        }

        ChipLogProgress(Zcl, "New brightness: %u | R: %u, G: %u, B: %u", sBrightness, sLedRgb.r, sLedRgb.g, sLedRgb.b);
        setRgbAction = true;
    }
    else if (aAction == COLOR_ACTION_XY)
    {
        sXY     = *reinterpret_cast<XyColor_t *>(value);
        sLedRgb = XYToRgb(sBrightness, sXY.x, sXY.y);
        ChipLogProgress(Zcl, "XY to RGB: X: %u, Y: %u, Level: %u | R: %u, G: %u, B: %u", sXY.x, sXY.y, sBrightness, sLedRgb.r,
                        sLedRgb.g, sLedRgb.b);
        setRgbAction = true;
        sColorAction = COLOR_ACTION_XY;
    }
    else if (aAction == COLOR_ACTION_HSV)
    {
        sHSV    = *reinterpret_cast<HsvColor_t *>(value);
        sHSV.v  = sBrightness;
        sLedRgb = HsvToRgb(sHSV);
        ChipLogProgress(Zcl, "HSV to RGB: H: %u, S: %u, V: %u | R: %u, G: %u, B: %u", sHSV.h, sHSV.s, sHSV.v, sLedRgb.r, sLedRgb.g,
                        sLedRgb.b);
        setRgbAction = true;
        sColorAction = COLOR_ACTION_HSV;
    }
    else if (aAction == COLOR_ACTION_CT)
    {
        sCT = *reinterpret_cast<CtColor_t *>(value);
        if (sCT.ctMireds)
        {
            sLedRgb = CTToRgb(sCT);
            ChipLogProgress(Zcl, "ColorTemp to RGB: CT: %u | R: %u, G: %u, B: %u", sCT.ctMireds, sLedRgb.r, sLedRgb.g, sLedRgb.b);
            setRgbAction = true;
            sColorAction = COLOR_ACTION_CT;
        }
    }

    if (setRgbAction)
    {
        PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Red, (((uint32_t) sLedRgb.r * 1000) / UINT8_MAX));
        PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Green, (((uint32_t) sLedRgb.g * 1000) / UINT8_MAX));
        PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Blue, (((uint32_t) sLedRgb.b * 1000) / UINT8_MAX));
    }
}

#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
static constexpr uint32_t kPowerOnFactoryResetIndicationMax    = 4;
static constexpr uint32_t kPowerOnFactoryResetIndicationTimeMs = 1000;

unsigned int AppTask::sPowerOnFactoryResetTimerCnt;
k_timer AppTask::sPowerOnFactoryResetTimer;

void AppTask::PowerOnFactoryResetEventHandler(AppEvent * aEvent)
{
    LOG_INF("Lighting App Power On Factory Reset Handler");
    sPowerOnFactoryResetTimerCnt = 1;
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Red, (bool) (sPowerOnFactoryResetTimerCnt % 2));
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Green, (bool) (sPowerOnFactoryResetTimerCnt % 2));
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Blue, (bool) (sPowerOnFactoryResetTimerCnt % 2));
#if !CONFIG_PWM
    LedManager::getInstance().setLed(LedManager::EAppLed_App0, (bool) (sPowerOnFactoryResetTimerCnt % 2));
#endif
    k_timer_init(&sPowerOnFactoryResetTimer, PowerOnFactoryResetTimerEvent, nullptr);
    k_timer_start(&sPowerOnFactoryResetTimer, K_MSEC(kPowerOnFactoryResetIndicationTimeMs),
                  K_MSEC(kPowerOnFactoryResetIndicationTimeMs));
}

void AppTask::PowerOnFactoryResetTimerEvent(struct k_timer * timer)
{
    sPowerOnFactoryResetTimerCnt++;
    LOG_INF("Lighting App Power On Factory Reset Handler %u", sPowerOnFactoryResetTimerCnt);
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Red, (bool) (sPowerOnFactoryResetTimerCnt % 2));
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Green, (bool) (sPowerOnFactoryResetTimerCnt % 2));
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Blue, (bool) (sPowerOnFactoryResetTimerCnt % 2));
    if (sPowerOnFactoryResetTimerCnt > kPowerOnFactoryResetIndicationMax)
    {
        k_timer_stop(timer);
        LOG_INF("schedule factory reset");
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */

void AppTask::LinkLeds(LedManager & ledManager)
{
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    ledManager.linkLed(LedManager::EAppLed_Status, 0);
#endif

#if !CONFIG_PWM
    ledManager.linkLed(LedManager::EAppLed_App0, 1);
#endif /* !CONFIG_PWM */
}
