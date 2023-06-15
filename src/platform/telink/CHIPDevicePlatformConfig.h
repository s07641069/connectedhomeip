/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
 *          Platform-specific configuration overrides for the chip Device Layer
 *          on Telink platform.
 */

#pragma once

#include "autoconf.h"

// ==================== Platform Adaptations ====================

#ifndef CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
#define CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER CONFIG_CHIP_DEVICE_SERIAL_NUMBER
#endif

#ifndef CHIP_DEVICE_CONFIG_TEST_MANUFACTURING_DATE
#define CHIP_DEVICE_CONFIG_TEST_MANUFACTURING_DATE CONFIG_CHIP_DEVICE_MANUFACTURING_DATE
#endif

#ifndef CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION CONFIG_CHIP_DEVICE_HARDWARE_VERSION
#endif

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE CONFIG_CHIP_DEVICE_SPAKE2_PASSCODE
#endif

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR CONFIG_CHIP_DEVICE_DISCRIMINATOR
#endif

#ifndef CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING CONFIG_CHIP_DEVICE_HARDWARE_VERSION_STRING
#endif

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT CONFIG_CHIP_DEVICE_SPAKE2_IT
#endif

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT CONFIG_CHIP_DEVICE_SPAKE2_SALT
#endif

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER CONFIG_CHIP_DEVICE_SPAKE2_TEST_VERIFIER
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID CONFIG_CHIP_DEVICE_VENDOR_ID
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID CONFIG_CHIP_DEVICE_PRODUCT_ID
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME CONFIG_CHIP_DEVICE_VENDOR_NAME
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME CONFIG_CHIP_DEVICE_PRODUCT_NAME
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION CONFIG_CHIP_DEVICE_SOFTWARE_VERSION
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING
#endif

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

#define CHIP_DEVICE_CONFIG_ENABLE_THREAD CONFIG_NET_L2_OPENTHREAD

#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE CONFIG_BT

// ========== Platform-specific Configuration =========

// These are configuration options that are unique to Zephyr platforms.
// These can be overridden by the application as needed.

#ifndef CHIP_DEVICE_CONFIG_SETTINGS_KEY
/// Key for all Matter persistent data stored using the Zephyr Settings API
#define CHIP_DEVICE_CONFIG_SETTINGS_KEY "mt"
#endif // CHIP_DEVICE_CONFIG_SETTINGS_KEY

#ifndef CHIP_DEVICE_CONFIG_OTA_REQUESTOR_REBOOT_DELAY_MS
/// Delay between completing a firmware update download and reboot to apply the update
#define CHIP_DEVICE_CONFIG_OTA_REQUESTOR_REBOOT_DELAY_MS 1000
#endif // CHIP_DEVICE_CONFIG_OTA_REQUESTOR_REBOOT_DELAY_MS

#ifndef CHIP_DEVICE_CONFIG_SERVER_SHUTDOWN_ACTIONS_SLEEP_MS
/// Time to sleep after running server shutdown actions to let lower layers complete the actions.
/// This may include transmitting packets created by the actions.
#define CHIP_DEVICE_CONFIG_SERVER_SHUTDOWN_ACTIONS_SLEEP_MS 500
#endif // CHIP_DEVICE_CONFIG_SERVER_SHUTDOWN_ACTIONS_SLEEP_MS

#ifndef CHIP_DEVICE_CONFIG_HEAP_STATISTICS_MALLINFO
#if !defined(CONFIG_CHIP_MALLOC_SYS_HEAP) && defined(CONFIG_NEWLIB_LIBC)
/// Use mallinfo() to obtain the heap usage statistics exposed by SoftwareDiagnostics cluster attributes.
#define CHIP_DEVICE_CONFIG_HEAP_STATISTICS_MALLINFO 1
#endif // !defined(CONFIG_CHIP_MALLOC_SYS_HEAP) && defined(CONFIG_NEWLIB_LIBC)
#endif // CHIP_DEVICE_CONFIG_HEAP_STATISTICS_MALLINFO

#ifndef CHIP_DEVICE_BLE_ADVERTISING_PRIORITY
/// Priority of the Matter BLE advertising when there are multiple application
/// components that compete for the BLE advertising.
#define CHIP_DEVICE_BLE_ADVERTISING_PRIORITY 0
#endif // CHIP_DEVICE_BLE_ADVERTISING_PRIORITY

#ifndef CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION
//-> format_version = 1
//-> vendor_id = 0x1141
//-> product_id = 0x8005
//-> device_type_id = 0x0100
//-> certificate_id = "ZIG20141ZB330001-24"
//-> security_level = 0
//-> security_information = 0
//-> version_number = 0x2694
//-> certification_type = 0
//-> dac_origin_vendor_id is not present
//-> dac_origin_product_id is not present
#define CHIP_DEVICE_CONFIG_CERTIFICATION_DECLARATION                                     \
    {                                                                                    \
        0x30, 0x81, 0xe9, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,          \
            0x07, 0x02, 0xa0, 0x81, 0xdb, 0x30, 0x81, 0xd8, 0x02, 0x01, 0x03, 0x31,      \
            0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04,      \
            0x02, 0x01, 0x30, 0x45, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,      \
            0x01, 0x07, 0x01, 0xa0, 0x38, 0x04, 0x36, 0x15, 0x24, 0x00, 0x01, 0x25,      \
            0x01, 0x41, 0x11, 0x36, 0x02, 0x05, 0x05, 0x80, 0x18, 0x25, 0x03, 0x00,      \
            0x01, 0x2c, 0x04, 0x13, 0x5a, 0x49, 0x47, 0x32, 0x30, 0x31, 0x34, 0x31,      \
            0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x31, 0x2d, 0x32, 0x34, 0x24,      \
            0x05, 0x00, 0x24, 0x06, 0x00, 0x25, 0x07, 0x76, 0x98, 0x24, 0x08, 0x00,      \
            0x18, 0x31, 0x7d, 0x30, 0x7b, 0x02, 0x01, 0x03, 0x80, 0x14, 0x6e, 0xf0,      \
            0xfd, 0xa7, 0x63, 0xf0, 0x64, 0x5b, 0xb8, 0xaf, 0xde, 0xab, 0x89, 0x52,      \
            0x8d, 0x26, 0x23, 0x0e, 0x5e, 0x81, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86,      \
            0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a,      \
            0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x04, 0x47, 0x30, 0x45, 0x02,      \
            0x20, 0x72, 0x4a, 0x80, 0x7b, 0xeb, 0x8a, 0x1c, 0x61, 0x6a, 0x32, 0x3a,      \
            0x98, 0x27, 0x46, 0x94, 0xdf, 0x12, 0x01, 0xb9, 0x82, 0x09, 0x9a, 0xb7,      \
            0x17, 0x52, 0x4e, 0xf5, 0x3a, 0x65, 0xec, 0x97, 0x1f, 0x02, 0x21, 0x00,      \
            0x93, 0x63, 0xd5, 0x9f, 0x83, 0x48, 0xc6, 0xc8, 0x35, 0xec, 0x51, 0x92,      \
            0xf8, 0xd3, 0x2b, 0x04, 0x69, 0xbe, 0xdb, 0x0e, 0x91, 0xe0, 0x26, 0x49,      \
            0x46, 0x2c, 0xa3, 0x53, 0x68, 0x98, 0xf9, 0x6f                               \
    }
#endif

// ========== Platform-specific Configuration Overrides =========

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY
#define CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY (K_PRIO_PREEMPT(1))
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0

#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 0

#ifndef CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 0
#else
#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART
#endif // CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART

#ifdef CONFIG_CHIP_ENABLE_DNSSD_SRP
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
#ifdef CONFIG_CHIP_ENABLE_DNS_CLIENT
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY 1
#endif // CONFIG_CHIP_ENABLE_DNS_CLIENT
#endif // CONFIG_CHIP_ENABLE_DNSSD_SRP

#ifdef CONFIG_CHIP_COMMISSIONABLE_DEVICE_TYPE
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1
#endif // CONFIG_CHIP_COMMISSIONABLE_DEVICE_TYPE

#ifdef CONFIG_CHIP_DEVICE_TYPE
#define CHIP_DEVICE_CONFIG_DEVICE_TYPE CONFIG_CHIP_DEVICE_TYPE
#endif // CONFIG_CHIP_DEVICE_TYPE

#ifdef CONFIG_CHIP_EXTENDED_DISCOVERY
#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1
#endif // CONFIG_CHIP_EXTENDED_DISCOVERY

#ifdef CONFIG_CHIP_ENABLE_SLEEPY_END_DEVICE_SUPPORT
#define CHIP_DEVICE_CONFIG_ENABLE_SED 1
#define CHIP_DEVICE_CONFIG_THREAD_SSED CONFIG_CHIP_THREAD_SSED
#endif // CONFIG_CHIP_ENABLE_SLEEPY_END_DEVICE_SUPPORT

#ifndef CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL
#ifdef CONFIG_CHIP_SED_IDLE_INTERVAL
#define CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL chip::System::Clock::Milliseconds32(CONFIG_CHIP_SED_IDLE_INTERVAL)
#endif // CONFIG_CHIP_SED_IDLE_INTERVAL
#endif // CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL

#ifndef CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL
#ifdef CONFIG_CHIP_SED_ACTIVE_INTERVAL
#define CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL chip::System::Clock::Milliseconds32(CONFIG_CHIP_SED_ACTIVE_INTERVAL)
#endif // CONFIG_CHIP_SED_ACTIVE_INTERVAL
#endif // CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL
