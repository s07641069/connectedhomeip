/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <zephyr/kernel.h>

#include <cstdint>

#include "AppEvent.h"

struct k_timer;

class ElectricalManager
{
public:
    int16_t GetDcVoltage();
    int16_t GetDcVoltageMin();
    int16_t GetDcVoltageMax();
    uint16_t GetDcVoltageMultiplier();
    uint16_t GetDcVoltageDivisor();

    int16_t GetDcCurrent();
    uint16_t GetDcCurrentMin();
    uint16_t GetDcCurrentMax();

    int16_t GetDcPower();
    int16_t GetDcPowerMin();
    int16_t GetDcPowerMax();
    uint16_t GetDcPowerMultiplier();
    uint16_t GetDcPowerDivisor();


    uint16_t GetAcFrequencyMultiplier();
    uint16_t GetAcPowerMultiplier();
    uint16_t GetAcPowerDivisor();
    int16_t GetInstantaneousVoltage();
    uint16_t GetAcFrequencyDivisor();
    uint16_t GetAcFrequency();
    uint16_t GetAcFrequencyMin();
    uint16_t GetAcFrequencyMax();
    uint16_t GetNeutralCurrent();
    int32_t GetTotalActivePower();
    int32_t GetTotalReactivePower();
    uint32_t GetTotalApparentPower();

private:
    friend ElectricalManager & ElectricalMgr(void);
    static ElectricalManager sOutlet;
};

inline ElectricalManager & ElectricalMgr(void)
{
    return ElectricalManager::sOutlet;
}
