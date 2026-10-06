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
    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_StartTagEmulation(const char * payload, size_t payloadLength)
{
    uint8_t* ndef_msg = new uint8_t[5 + payloadLength];
    size_t ndef_msg_len = _EncodeNDEFURI((const uint8_t *) payload, payloadLength, ndef_msg);
    int result = st25dv_type5_tag_add(mpNFCDevice, ndef_msg, ndef_msg_len);

    delete[] ndef_msg;
    if (result)
    {
        ChipLogError(DeviceLayer, "NFC can't write tag %d", result);
        return CHIP_ERROR_INTERNAL;
    }
    ChipLogDetail(DeviceLayer, "NFC tag written");
    sInstance.mIsStarted = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCOnboardingPayloadManagerImpl::_StopTagEmulation()
{
    int result = st25dv_type5_tag_rm(mpNFCDevice);

    if (result)
    {
        ChipLogError(DeviceLayer, "NFC can't erase tag %d", result);
        return CHIP_ERROR_INTERNAL;
    }
    ChipLogDetail(DeviceLayer, "NFC tag erased");
    sInstance.mIsStarted = false;
    return CHIP_NO_ERROR;
}

size_t NFCOnboardingPayloadManagerImpl::_EncodeNDEFURI(const uint8_t * uri, size_t uri_len, uint8_t * out_buf)
{
    size_t max_len = 128;
    // NDEF Record Header for short record, TNF = Well Known, Type = 'U'
    const uint8_t type_field     = 'U';
    const uint8_t tnf            = 0xD1; // MB=1, ME=1, SR=1, TNF=1
    const uint8_t payload_prefix = 0x00; // No prefix, full URI follows

    if (max_len < 5 + uri_len)
    {
        ChipLogError(DeviceLayer, "NDEF message is too long, (5 + uri_len) required for TNF URI NDEF record");
        return 0;
    }

    size_t offset     = 0;
    out_buf[offset++] = tnf;
    out_buf[offset++] = 1;           // Type length
    out_buf[offset++] = uri_len + 1; // Payload length (1 prefix + uri)
    out_buf[offset++] = type_field;
    out_buf[offset++] = payload_prefix;
    memcpy(out_buf + offset, uri, uri_len);
    offset += uri_len;

    return offset;
}

} // namespace DeviceLayer
} // namespace chip
