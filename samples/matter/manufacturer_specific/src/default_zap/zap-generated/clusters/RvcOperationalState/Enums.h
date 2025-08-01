/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

// THIS FILE IS GENERATED BY ZAP
// This file is generated from clusters-Enums.h.zapt

#pragma once

#include <clusters/shared/Enums.h>
#include <cstdint>

namespace chip
{
namespace app
{
	namespace Clusters
	{
		namespace RvcOperationalState
		{

			// Enum for ErrorStateEnum
			enum class ErrorStateEnum : uint8_t {
				kNoError = 0x00,
				kUnableToStartOrResume = 0x01,
				kUnableToCompleteOperation = 0x02,
				kCommandInvalidInState = 0x03,
				kFailedToFindChargingDock = 0x40,
				kStuck = 0x41,
				kDustBinMissing = 0x42,
				kDustBinFull = 0x43,
				kWaterTankEmpty = 0x44,
				kWaterTankMissing = 0x45,
				kWaterTankLidOpen = 0x46,
				kMopCleaningPadMissing = 0x47,
				kLowBattery = 0x48,
				kCannotReachTargetArea = 0x49,
				kDirtyWaterTankFull = 0x4A,
				kDirtyWaterTankMissing = 0x4B,
				kWheelsJammed = 0x4C,
				kBrushJammed = 0x4D,
				kNavigationSensorObscured = 0x4E,
				// kUnknownEnumValue intentionally not defined. This enum never goes
				// through DataModel::Decode, likely because it is a part of a derived
				// cluster. As a result having kUnknownEnumValue in this enum is error
				// prone, and was removed. See
				// src/app/common/templates/config-data.yaml.
			};

			// Enum for OperationalStateEnum
			enum class OperationalStateEnum : uint8_t {
				kStopped = 0x00,
				kRunning = 0x01,
				kPaused = 0x02,
				kError = 0x03,
				kSeekingCharger = 0x40,
				kCharging = 0x41,
				kDocked = 0x42,
				kEmptyingDustBin = 0x43,
				kCleaningMop = 0x44,
				kFillingWaterTank = 0x45,
				kUpdatingMaps = 0x46,
				// kUnknownEnumValue intentionally not defined. This enum never goes
				// through DataModel::Decode, likely because it is a part of a derived
				// cluster. As a result having kUnknownEnumValue in this enum is error
				// prone, and was removed. See
				// src/app/common/templates/config-data.yaml.
			};
		} // namespace RvcOperationalState
	} // namespace Clusters
} // namespace app
} // namespace chip
