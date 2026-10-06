/*
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/NFCOnboardingPayloadManager.h>

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {

NFCOnboardingPayloadManagerImpl NFCOnboardingPayloadManagerImpl::sInstance;

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_Init()
{
    int result;

    do
    {
        result = st25dv_init(mpNFCDevice);
        if (result)
        {
            ChipLogError(DeviceLayer, "NFC init failed %d", result);
            break;
        }
        mIsStarted = false;
        ChipLogDetail(DeviceLayer, "NFC tag inited");
    } while (0);
    return result ? CHIP_ERROR_INTERNAL : CHIP_NO_ERROR;
}

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_StartTagEmulation(const char * payload, size_t payloadLength)
{
    int result = -EALREADY;

    do
    {
        if (mIsStarted)
        {
            ChipLogError(DeviceLayer, "NFC tag already enabled");
            break;
        }

        uint8_t * ndef_msg = new uint8_t[8 + payloadLength];

        ndef_msg[0] = 0xc1;                                           // record header (MB=1, ME=1, CF=0, SR=0, IL=0, TNF=001)
        ndef_msg[1] = 1;                                              // length of record type
        ndef_msg[2] = (uint8_t) (((payloadLength + 1) >> 24) & 0xff); // length
        ndef_msg[3] = (uint8_t) (((payloadLength + 1) >> 16) & 0xff); // length
        ndef_msg[4] = (uint8_t) (((payloadLength + 1) >> 8) & 0xff);  // length
        ndef_msg[5] = (uint8_t) ((payloadLength + 1) & 0xff);         // length
        ndef_msg[6] = (uint8_t) 'U';                                  // record Type
        ndef_msg[7] = 0x00;                                           // uri prefix code (no prefix)
        memcpy(&ndef_msg[8], payload, payloadLength);
        result = st25dv_tag_type5_add(mpNFCDevice, ndef_msg, 8 + payloadLength);
        delete[] ndef_msg;
        if (result)
        {
            ChipLogError(DeviceLayer, "NFC can't write tag %d", result);
            break;
        }
        result = st25dv_mode_set(mpNFCDevice, ST25DV_MODE_TAG);
        if (result)
        {
            ChipLogError(DeviceLayer, "NFC enabling tag failed %d", result);
            break;
        }
        mIsStarted = true;
        ChipLogDetail(DeviceLayer, "NFC tag written and enabled");

    } while (0);
    return result ? CHIP_ERROR_INTERNAL : CHIP_NO_ERROR;
}

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_StopTagEmulation()
{
    int result = 0;

    do
    {
        if (!mIsStarted)
        {
            break;
        }
        result = st25dv_tag_type5_rm(mpNFCDevice);
        if (result)
        {
            ChipLogError(DeviceLayer, "NFC can't erase tag %d", result);
            break;
        }
        result = st25dv_mode_set(mpNFCDevice, ST25DV_MODE_OFF);
        if (result)
        {
            ChipLogError(DeviceLayer, "NFC disabling tag failed %d", result);
            break;
        }
        mIsStarted = false;
        ChipLogDetail(DeviceLayer, "NFC tag erased and disabled");
    } while (0);
    return result ? CHIP_ERROR_INTERNAL : CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
