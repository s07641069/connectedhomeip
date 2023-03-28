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

#include "ElectricalManager.h"

#include "AppConfig.h"
#include "AppTask.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

ElectricalManager ElectricalManager::sOutlet;

int16_t ElectricalManager::GetDcVoltage()
{
    return 100;
}

int16_t ElectricalManager::GetDcVoltageMin()
{
    return 0;
}

int16_t ElectricalManager::GetDcVoltageMax()
{
    return 200;
}

int16_t ElectricalManager::GetDcCurrent()
{
    return 10;
}

uint16_t ElectricalManager::GetDcCurrentMin()
{
    return 0;
}

uint16_t ElectricalManager::GetDcCurrentMax()
{
    return 20;
}

int16_t ElectricalManager::GetDcPower()
{
    return 500;
}

int16_t ElectricalManager::GetDcPowerMin()
{
    return 0;
}

int16_t ElectricalManager::GetDcPowerMax()
{
    return 4000;
}

uint16_t ElectricalManager::GetDcVoltageMultiplier()
{
    return 1;
}

uint16_t ElectricalManager::GetDcVoltageDivisor()
{
    return 1;
}

uint16_t ElectricalManager::GetDcPowerMultiplier()
{
    return 1;
}

uint16_t ElectricalManager::GetDcPowerDivisor()
{
    return 1;
}

// AC

uint16_t ElectricalManager::GetAcFrequencyMultiplier()
{
    return 1;
}

uint16_t ElectricalManager::GetAcFrequencyDivisor()
{
    return 1;
}

uint16_t ElectricalManager::GetAcPowerMultiplier()
{
    return 1;
}

uint16_t ElectricalManager::GetAcPowerDivisor()
{
    return 1;
}

int16_t ElectricalManager::GetInstantaneousVoltage()
{
    return 220;
}

uint16_t ElectricalManager::GetAcFrequency()
{
    return 50;
}

uint16_t ElectricalManager::GetAcFrequencyMin()
{
    return 1;
}

uint16_t ElectricalManager::GetAcFrequencyMax()
{
    return 60;
}

uint16_t ElectricalManager::GetNeutralCurrent()
{
    return 5;
}

int32_t ElectricalManager::GetTotalActivePower()
{
    return 300;
}

int32_t ElectricalManager::GetTotalReactivePower()
{
    return 111;
}

uint32_t ElectricalManager::GetTotalApparentPower()
{
    return 100;
}
