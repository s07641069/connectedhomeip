/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "AliroDelegate.h"

#include <AppConfig.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/Span.h>

using namespace chip;
using namespace chip::app::Clusters::DoorLock;

AliroDelegate::AliroDelegate()
{
    // The group sub-identifier is non-nullable; initialize once with random bytes.
    TEMPORARY_RETURN_IGNORED Crypto::DRBG_get_bytes(mAliroReaderGroupSubIdentifier, sizeof(mAliroReaderGroupSubIdentifier));
}

CHIP_ERROR AliroDelegate::GetAliroReaderVerificationKey(MutableByteSpan & verificationKey)
{
    if (!mAliroStateInitialized)
    {
        verificationKey.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    return CopySpanToMutableSpan(ByteSpan(mAliroReaderVerificationKey), verificationKey);
}

CHIP_ERROR AliroDelegate::GetAliroReaderGroupIdentifier(MutableByteSpan & groupIdentifier)
{
    if (!mAliroStateInitialized)
    {
        groupIdentifier.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    return CopySpanToMutableSpan(ByteSpan(mAliroReaderGroupIdentifier), groupIdentifier);
}

CHIP_ERROR AliroDelegate::GetAliroReaderGroupSubIdentifier(MutableByteSpan & groupSubIdentifier)
{
    return CopySpanToMutableSpan(ByteSpan(mAliroReaderGroupSubIdentifier), groupSubIdentifier);
}

CHIP_ERROR AliroDelegate::CopyProtocolVersionIntoSpan(uint16_t protocolVersionValue, MutableByteSpan & protocolVersion)
{
    static_assert(sizeof(protocolVersionValue) == kAliroProtocolVersionSize);

    if (protocolVersion.size() < kAliroProtocolVersionSize)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Per Aliro spec, protocol version encoding is big-endian.
    Encoding::BigEndian::Put16(protocolVersion.data(), protocolVersionValue);
    protocolVersion.reduce_size(kAliroProtocolVersionSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AliroDelegate::GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index,
                                                                                      MutableByteSpan & protocolVersion)
{
    // Only claim support for the one known protocol version for now: 0x0100.
    constexpr uint16_t knownProtocolVersion = 0x0100;

    if (index > 0)
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    return CopyProtocolVersionIntoSpan(knownProtocolVersion, protocolVersion);
}

CHIP_ERROR AliroDelegate::GetAliroGroupResolvingKey(MutableByteSpan & groupResolvingKey)
{
    if (!mAliroStateInitialized)
    {
        groupResolvingKey.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    return CopySpanToMutableSpan(ByteSpan(mAliroGroupResolvingKey), groupResolvingKey);
}

CHIP_ERROR AliroDelegate::GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, MutableByteSpan & protocolVersion)
{
    // Only claim support for the one known protocol version for now: 0x0100.
    constexpr uint16_t knownProtocolVersion = 0x0100;

    if (index > 0)
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    return CopyProtocolVersionIntoSpan(knownProtocolVersion, protocolVersion);
}

uint8_t AliroDelegate::GetAliroBLEAdvertisingVersion()
{
    // For now the only defined value of the BLE advertising version for Aliro is 0.
    return 0;
}

uint16_t AliroDelegate::GetNumberOfAliroCredentialIssuerKeysSupported()
{
    return APP_MAX_CREDENTIAL;
}

uint16_t AliroDelegate::GetNumberOfAliroEndpointKeysSupported()
{
    return APP_MAX_CREDENTIAL;
}

CHIP_ERROR AliroDelegate::SetAliroReaderConfig(const ByteSpan & signingKey, const ByteSpan & verificationKey,
                                               const ByteSpan & groupIdentifier,
                                               const Optional<ByteSpan> & groupResolvingKey)
{
    // We ignore the signing key, since we never do anything with it.
    (void) signingKey;

    VerifyOrReturnError(verificationKey.size() == sizeof(mAliroReaderVerificationKey), CHIP_ERROR_INVALID_ARGUMENT);
    memcpy(mAliroReaderVerificationKey, verificationKey.data(), sizeof(mAliroReaderVerificationKey));

    VerifyOrReturnError(groupIdentifier.size() == sizeof(mAliroReaderGroupIdentifier), CHIP_ERROR_INVALID_ARGUMENT);
    memcpy(mAliroReaderGroupIdentifier, groupIdentifier.data(), sizeof(mAliroReaderGroupIdentifier));

    if (groupResolvingKey.HasValue())
    {
        VerifyOrReturnError(groupResolvingKey.Value().size() == sizeof(mAliroGroupResolvingKey), CHIP_ERROR_INVALID_ARGUMENT);
        memcpy(mAliroGroupResolvingKey, groupResolvingKey.Value().data(), sizeof(mAliroGroupResolvingKey));
    }

    mAliroStateInitialized = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR AliroDelegate::ClearAliroReaderConfig()
{
    // A real implementation would clear out key data from persistent storage.
    mAliroStateInitialized = false;
    return CHIP_NO_ERROR;
}
