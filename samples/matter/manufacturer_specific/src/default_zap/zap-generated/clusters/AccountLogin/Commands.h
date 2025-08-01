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
// This file is generated from clusters-Commands.h.zapt

#pragma once

#include <app/data-model/DecodableList.h>
#include <app/data-model/Encode.h>
#include <app/data-model/List.h>
#include <app/data-model/NullObject.h>
#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/core/TLV.h>
#include <lib/support/BitMask.h>

#include <clusters/shared/Enums.h>
#include <clusters/shared/Structs.h>

#include <clusters/AccountLogin/ClusterId.h>
#include <clusters/AccountLogin/CommandIds.h>
#include <clusters/AccountLogin/Enums.h>
#include <clusters/AccountLogin/Structs.h>

#include <cstdint>

namespace chip
{
namespace app
{
	namespace Clusters
	{
		namespace AccountLogin
		{
			namespace Commands
			{
				// Forward-declarations so we can reference these later.

				namespace GetSetupPIN
				{
					struct Type;
					struct DecodableType;
				} // namespace GetSetupPIN

				namespace GetSetupPINResponse
				{
					struct Type;
					struct DecodableType;
				} // namespace GetSetupPINResponse

				namespace Login
				{
					struct Type;
					struct DecodableType;
				} // namespace Login

				namespace Logout
				{
					struct Type;
					struct DecodableType;
				} // namespace Logout

			} // namespace Commands

			namespace Commands
			{
				namespace GetSetupPIN
				{
					enum class Fields : uint8_t {
						kTempAccountIdentifier = 0,
					};

					struct Type {
					public:
						// Use GetCommandId instead of commandId directly to avoid naming
						// conflict with CommandIdentification in ExecutionOfACommand
						static constexpr CommandId GetCommandId()
						{
							return Commands::GetSetupPIN::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::AccountLogin::Id;
						}

						chip::CharSpan tempAccountIdentifier;

						CHIP_ERROR Encode(TLV::TLVWriter &aWriter, TLV::Tag aTag) const;

						using ResponseType = Clusters::AccountLogin::Commands::
							GetSetupPINResponse::DecodableType;

						static constexpr bool MustUseTimedInvoke() { return true; }
					};

					struct DecodableType {
					public:
						static constexpr CommandId GetCommandId()
						{
							return Commands::GetSetupPIN::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::AccountLogin::Id;
						}
						static constexpr bool kIsFabricScoped = true;

						chip::CharSpan tempAccountIdentifier;

						CHIP_ERROR Decode(TLV::TLVReader &reader,
								  FabricIndex aAccessingFabricIndex);
					};
				}; // namespace GetSetupPIN
				namespace GetSetupPINResponse
				{
					enum class Fields : uint8_t {
						kSetupPIN = 0,
					};

					struct Type {
					public:
						// Use GetCommandId instead of commandId directly to avoid naming
						// conflict with CommandIdentification in ExecutionOfACommand
						static constexpr CommandId GetCommandId()
						{
							return Commands::GetSetupPINResponse::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::AccountLogin::Id;
						}

						chip::CharSpan setupPIN;

						CHIP_ERROR Encode(DataModel::FabricAwareTLVWriter &aWriter,
								  TLV::Tag aTag) const;

						using ResponseType = DataModel::NullObjectType;

						static constexpr bool MustUseTimedInvoke() { return false; }
					};

					struct DecodableType {
					public:
						static constexpr CommandId GetCommandId()
						{
							return Commands::GetSetupPINResponse::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::AccountLogin::Id;
						}

						chip::CharSpan setupPIN;

						CHIP_ERROR Decode(TLV::TLVReader &reader);
					};
				}; // namespace GetSetupPINResponse
				namespace Login
				{
					enum class Fields : uint8_t {
						kTempAccountIdentifier = 0,
						kSetupPIN = 1,
						kNode = 2,
					};

					struct Type {
					public:
						// Use GetCommandId instead of commandId directly to avoid naming
						// conflict with CommandIdentification in ExecutionOfACommand
						static constexpr CommandId GetCommandId()
						{
							return Commands::Login::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::AccountLogin::Id;
						}

						chip::CharSpan tempAccountIdentifier;
						chip::CharSpan setupPIN;
						Optional<chip::NodeId> node;

						CHIP_ERROR Encode(TLV::TLVWriter &aWriter, TLV::Tag aTag) const;

						using ResponseType = DataModel::NullObjectType;

						static constexpr bool MustUseTimedInvoke() { return true; }
					};

					struct DecodableType {
					public:
						static constexpr CommandId GetCommandId()
						{
							return Commands::Login::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::AccountLogin::Id;
						}
						static constexpr bool kIsFabricScoped = true;

						chip::CharSpan tempAccountIdentifier;
						chip::CharSpan setupPIN;
						Optional<chip::NodeId> node;

						CHIP_ERROR Decode(TLV::TLVReader &reader,
								  FabricIndex aAccessingFabricIndex);
					};
				}; // namespace Login
				namespace Logout
				{
					enum class Fields : uint8_t {
						kNode = 0,
					};

					struct Type {
					public:
						// Use GetCommandId instead of commandId directly to avoid naming
						// conflict with CommandIdentification in ExecutionOfACommand
						static constexpr CommandId GetCommandId()
						{
							return Commands::Logout::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::AccountLogin::Id;
						}

						Optional<chip::NodeId> node;

						CHIP_ERROR Encode(TLV::TLVWriter &aWriter, TLV::Tag aTag) const;

						using ResponseType = DataModel::NullObjectType;

						static constexpr bool MustUseTimedInvoke() { return true; }
					};

					struct DecodableType {
					public:
						static constexpr CommandId GetCommandId()
						{
							return Commands::Logout::Id;
						}
						static constexpr ClusterId GetClusterId()
						{
							return Clusters::AccountLogin::Id;
						}
						static constexpr bool kIsFabricScoped = true;

						Optional<chip::NodeId> node;

						CHIP_ERROR Decode(TLV::TLVReader &reader,
								  FabricIndex aAccessingFabricIndex);
					};
				}; // namespace Logout
			} // namespace Commands
		} // namespace AccountLogin
	} // namespace Clusters
} // namespace app
} // namespace chip
