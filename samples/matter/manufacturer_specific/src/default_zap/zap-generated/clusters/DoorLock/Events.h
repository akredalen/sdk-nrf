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
// This file is generated from clusters-Events.h.zapt

#pragma once

#include <app/EventLoggingTypes.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/List.h>
#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLV.h>
#include <lib/support/BitMask.h>

#include <clusters/shared/Enums.h>
#include <clusters/shared/Structs.h>

#include <clusters/DoorLock/ClusterId.h>
#include <clusters/DoorLock/Enums.h>
#include <clusters/DoorLock/EventIds.h>
#include <clusters/DoorLock/Structs.h>

#include <cstdint>

namespace chip
{
namespace app
{
	namespace Clusters
	{
		namespace DoorLock
		{
			namespace Events
			{
				namespace DoorLockAlarm
				{
					static constexpr PriorityLevel kPriorityLevel = PriorityLevel::Critical;

					enum class Fields : uint8_t {
						kAlarmCode = 0,
					};

					struct Type {
					public:
						static constexpr PriorityLevel GetPriorityLevel()
						{
							return kPriorityLevel;
						}
						static constexpr EventId GetEventId()
						{
							return Events::DoorLockAlarm::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::DoorLock::Id;
						}
						static constexpr bool kIsFabricScoped = false;

						AlarmCodeEnum alarmCode = static_cast<AlarmCodeEnum>(0);

						CHIP_ERROR Encode(TLV::TLVWriter &aWriter, TLV::Tag aTag) const;
					};

					struct DecodableType {
					public:
						static constexpr PriorityLevel GetPriorityLevel()
						{
							return kPriorityLevel;
						}
						static constexpr EventId GetEventId()
						{
							return Events::DoorLockAlarm::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::DoorLock::Id;
						}

						AlarmCodeEnum alarmCode = static_cast<AlarmCodeEnum>(0);

						CHIP_ERROR Decode(TLV::TLVReader &reader);
					};
				} // namespace DoorLockAlarm
				namespace DoorStateChange
				{
					static constexpr PriorityLevel kPriorityLevel = PriorityLevel::Critical;

					enum class Fields : uint8_t {
						kDoorState = 0,
					};

					struct Type {
					public:
						static constexpr PriorityLevel GetPriorityLevel()
						{
							return kPriorityLevel;
						}
						static constexpr EventId GetEventId()
						{
							return Events::DoorStateChange::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::DoorLock::Id;
						}
						static constexpr bool kIsFabricScoped = false;

						DoorStateEnum doorState = static_cast<DoorStateEnum>(0);

						CHIP_ERROR Encode(TLV::TLVWriter &aWriter, TLV::Tag aTag) const;
					};

					struct DecodableType {
					public:
						static constexpr PriorityLevel GetPriorityLevel()
						{
							return kPriorityLevel;
						}
						static constexpr EventId GetEventId()
						{
							return Events::DoorStateChange::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::DoorLock::Id;
						}

						DoorStateEnum doorState = static_cast<DoorStateEnum>(0);

						CHIP_ERROR Decode(TLV::TLVReader &reader);
					};
				} // namespace DoorStateChange
				namespace LockOperation
				{
					static constexpr PriorityLevel kPriorityLevel = PriorityLevel::Critical;

					enum class Fields : uint8_t {
						kLockOperationType = 0,
						kOperationSource = 1,
						kUserIndex = 2,
						kFabricIndex = 3,
						kSourceNode = 4,
						kCredentials = 5,
					};

					struct Type {
					public:
						static constexpr PriorityLevel GetPriorityLevel()
						{
							return kPriorityLevel;
						}
						static constexpr EventId GetEventId()
						{
							return Events::LockOperation::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::DoorLock::Id;
						}
						static constexpr bool kIsFabricScoped = false;

						LockOperationTypeEnum lockOperationType =
							static_cast<LockOperationTypeEnum>(0);
						OperationSourceEnum operationSource =
							static_cast<OperationSourceEnum>(0);
						DataModel::Nullable<uint16_t> userIndex;
						DataModel::Nullable<chip::FabricIndex> fabricIndex;
						DataModel::Nullable<chip::NodeId> sourceNode;
						Optional<DataModel::Nullable<
							DataModel::List<const Structs::CredentialStruct::Type>>>
							credentials;

						CHIP_ERROR Encode(TLV::TLVWriter &aWriter, TLV::Tag aTag) const;
					};

					struct DecodableType {
					public:
						static constexpr PriorityLevel GetPriorityLevel()
						{
							return kPriorityLevel;
						}
						static constexpr EventId GetEventId()
						{
							return Events::LockOperation::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::DoorLock::Id;
						}

						LockOperationTypeEnum lockOperationType =
							static_cast<LockOperationTypeEnum>(0);
						OperationSourceEnum operationSource =
							static_cast<OperationSourceEnum>(0);
						DataModel::Nullable<uint16_t> userIndex;
						DataModel::Nullable<chip::FabricIndex> fabricIndex;
						DataModel::Nullable<chip::NodeId> sourceNode;
						Optional<DataModel::Nullable<DataModel::DecodableList<
							Structs::CredentialStruct::DecodableType>>>
							credentials;

						CHIP_ERROR Decode(TLV::TLVReader &reader);
					};
				} // namespace LockOperation
				namespace LockOperationError
				{
					static constexpr PriorityLevel kPriorityLevel = PriorityLevel::Critical;

					enum class Fields : uint8_t {
						kLockOperationType = 0,
						kOperationSource = 1,
						kOperationError = 2,
						kUserIndex = 3,
						kFabricIndex = 4,
						kSourceNode = 5,
						kCredentials = 6,
					};

					struct Type {
					public:
						static constexpr PriorityLevel GetPriorityLevel()
						{
							return kPriorityLevel;
						}
						static constexpr EventId GetEventId()
						{
							return Events::LockOperationError::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::DoorLock::Id;
						}
						static constexpr bool kIsFabricScoped = false;

						LockOperationTypeEnum lockOperationType =
							static_cast<LockOperationTypeEnum>(0);
						OperationSourceEnum operationSource =
							static_cast<OperationSourceEnum>(0);
						OperationErrorEnum operationError = static_cast<OperationErrorEnum>(0);
						DataModel::Nullable<uint16_t> userIndex;
						DataModel::Nullable<chip::FabricIndex> fabricIndex;
						DataModel::Nullable<chip::NodeId> sourceNode;
						Optional<DataModel::Nullable<
							DataModel::List<const Structs::CredentialStruct::Type>>>
							credentials;

						CHIP_ERROR Encode(TLV::TLVWriter &aWriter, TLV::Tag aTag) const;
					};

					struct DecodableType {
					public:
						static constexpr PriorityLevel GetPriorityLevel()
						{
							return kPriorityLevel;
						}
						static constexpr EventId GetEventId()
						{
							return Events::LockOperationError::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::DoorLock::Id;
						}

						LockOperationTypeEnum lockOperationType =
							static_cast<LockOperationTypeEnum>(0);
						OperationSourceEnum operationSource =
							static_cast<OperationSourceEnum>(0);
						OperationErrorEnum operationError = static_cast<OperationErrorEnum>(0);
						DataModel::Nullable<uint16_t> userIndex;
						DataModel::Nullable<chip::FabricIndex> fabricIndex;
						DataModel::Nullable<chip::NodeId> sourceNode;
						Optional<DataModel::Nullable<DataModel::DecodableList<
							Structs::CredentialStruct::DecodableType>>>
							credentials;

						CHIP_ERROR Decode(TLV::TLVReader &reader);
					};
				} // namespace LockOperationError
				namespace LockUserChange
				{
					static constexpr PriorityLevel kPriorityLevel = PriorityLevel::Info;

					enum class Fields : uint8_t {
						kLockDataType = 0,
						kDataOperationType = 1,
						kOperationSource = 2,
						kUserIndex = 3,
						kFabricIndex = 4,
						kSourceNode = 5,
						kDataIndex = 6,
					};

					struct Type {
					public:
						static constexpr PriorityLevel GetPriorityLevel()
						{
							return kPriorityLevel;
						}
						static constexpr EventId GetEventId()
						{
							return Events::LockUserChange::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::DoorLock::Id;
						}
						static constexpr bool kIsFabricScoped = false;

						LockDataTypeEnum lockDataType = static_cast<LockDataTypeEnum>(0);
						DataOperationTypeEnum dataOperationType =
							static_cast<DataOperationTypeEnum>(0);
						OperationSourceEnum operationSource =
							static_cast<OperationSourceEnum>(0);
						DataModel::Nullable<uint16_t> userIndex;
						DataModel::Nullable<chip::FabricIndex> fabricIndex;
						DataModel::Nullable<chip::NodeId> sourceNode;
						DataModel::Nullable<uint16_t> dataIndex;

						CHIP_ERROR Encode(TLV::TLVWriter &aWriter, TLV::Tag aTag) const;
					};

					struct DecodableType {
					public:
						static constexpr PriorityLevel GetPriorityLevel()
						{
							return kPriorityLevel;
						}
						static constexpr EventId GetEventId()
						{
							return Events::LockUserChange::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::DoorLock::Id;
						}

						LockDataTypeEnum lockDataType = static_cast<LockDataTypeEnum>(0);
						DataOperationTypeEnum dataOperationType =
							static_cast<DataOperationTypeEnum>(0);
						OperationSourceEnum operationSource =
							static_cast<OperationSourceEnum>(0);
						DataModel::Nullable<uint16_t> userIndex;
						DataModel::Nullable<chip::FabricIndex> fabricIndex;
						DataModel::Nullable<chip::NodeId> sourceNode;
						DataModel::Nullable<uint16_t> dataIndex;

						CHIP_ERROR Decode(TLV::TLVReader &reader);
					};
				} // namespace LockUserChange
			} // namespace Events
		} // namespace DoorLock
	} // namespace Clusters
} // namespace app
} // namespace chip
