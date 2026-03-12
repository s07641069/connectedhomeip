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

#pragma once

#include <app/clusters/door-lock-server/door-lock-delegate.h>

class AliroDelegate : public chip::app::Clusters::DoorLock::Delegate
{
public:
    AliroDelegate();

    CHIP_ERROR GetAliroReaderVerificationKey(chip::MutableByteSpan & verificationKey) override;
    CHIP_ERROR GetAliroReaderGroupIdentifier(chip::MutableByteSpan & groupIdentifier) override;
    CHIP_ERROR GetAliroReaderGroupSubIdentifier(chip::MutableByteSpan & groupSubIdentifier) override;
    CHIP_ERROR GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index,
                                                                           chip::MutableByteSpan & protocolVersion) override;
    CHIP_ERROR GetAliroGroupResolvingKey(chip::MutableByteSpan & groupResolvingKey) override;
    CHIP_ERROR GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, chip::MutableByteSpan & protocolVersion) override;
    uint8_t GetAliroBLEAdvertisingVersion() override;
    uint16_t GetNumberOfAliroCredentialIssuerKeysSupported() override;
    uint16_t GetNumberOfAliroEndpointKeysSupported() override;
    CHIP_ERROR SetAliroReaderConfig(const chip::ByteSpan & signingKey, const chip::ByteSpan & verificationKey,
                                    const chip::ByteSpan & groupIdentifier,
                                    const chip::Optional<chip::ByteSpan> & groupResolvingKey) override;
    CHIP_ERROR ClearAliroReaderConfig() override;

private:
    CHIP_ERROR CopyProtocolVersionIntoSpan(uint16_t protocolVersionValue, chip::MutableByteSpan & protocolVersion);

    uint8_t mAliroReaderVerificationKey[chip::app::Clusters::DoorLock::kAliroReaderVerificationKeySize];
    uint8_t mAliroReaderGroupIdentifier[chip::app::Clusters::DoorLock::kAliroReaderGroupIdentifierSize];
    uint8_t mAliroReaderGroupSubIdentifier[chip::app::Clusters::DoorLock::kAliroReaderGroupSubIdentifierSize];
    uint8_t mAliroGroupResolvingKey[chip::app::Clusters::DoorLock::kAliroGroupResolvingKeySize];
    bool mAliroStateInitialized = false;
};
