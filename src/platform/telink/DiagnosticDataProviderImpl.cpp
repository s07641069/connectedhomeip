/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright 2023, 2025 NXP
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

/**
 *    @file
 *          Provides an implementation of the DiagnosticDataProvider object
 *          for nxp platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "DiagnosticDataProviderImpl.h"
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DiagnosticDataProvider.h>

#include <inet/InetInterface.h>
#include <openthread/ip6.h>

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    NetworkInterface * ifp = new NetworkInterface();

// #if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    const char * threadNetworkName = otThreadGetNetworkName(ThreadStackMgrImpl().OTInstance());
    ifp->name                      = CharSpan(threadNetworkName, strlen(threadNetworkName));
    ifp->isOperational             = true;
    ifp->offPremiseServicesReachableIPv4.SetNull();
    ifp->offPremiseServicesReachableIPv6.SetNull();
    ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kThread;
    TEMPORARY_RETURN_IGNORED ConfigurationMgr().GetPrimary802154MACAddress(ifp->MacAddress);
    ifp->hardwareAddress = ByteSpan(ifp->MacAddress, kMaxHardwareAddrSize);
// #endif

    // IPv6-only support: use OpenThread unicast addresses directly.
    uint8_t ipv6AddressesCount = 0;
    if (otInstance * otInst = ThreadStackMgrImpl().OTInstance())
    {
        for (const otNetifAddress * addr = otIp6GetUnicastAddresses(otInst);
             addr != nullptr && ipv6AddressesCount < kMaxIPv6AddrCount; addr = addr->mNext)
        {
            memcpy(ifp->Ipv6AddressesBuffer[ipv6AddressesCount], addr->mAddress.mFields.m8, kMaxIPv6AddrSize);
            ifp->Ipv6AddressSpans[ipv6AddressesCount] =
                ByteSpan(ifp->Ipv6AddressesBuffer[ipv6AddressesCount], kMaxIPv6AddrSize);
            {
                char ipStr[40];
                const uint8_t * b = addr->mAddress.mFields.m8;
                snprintf(ipStr, sizeof(ipStr),
                         "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", b[0], b[1],
                         b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14],
                         b[15]);
                ChipLogProgress(DeviceLayer, "Diag: IPv6[%u]=%s", static_cast<unsigned>(ipv6AddressesCount), ipStr);
            }
            ipv6AddressesCount++;
        }
        ChipLogProgress(DeviceLayer, "Diag: IPv6 address count=%u", static_cast<unsigned>(ipv6AddressesCount));
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Diag: OT instance is null");
    }
    ifp->IPv6Addresses = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, ipv6AddressesCount);

    *netifpp = ifp;
    return CHIP_NO_ERROR;
}

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
