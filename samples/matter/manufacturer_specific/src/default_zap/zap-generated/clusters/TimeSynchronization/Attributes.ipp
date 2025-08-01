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
// This file is generated from clusters-Attributes.ipp.zapt

#include <clusters/TimeSynchronization/Attributes.h>

#include <app/data-model/Decode.h>
#include <app/data-model/WrappedStructEncoder.h>

namespace chip
{
namespace app
{
	namespace Clusters
	{
		namespace TimeSynchronization
		{
			namespace Attributes
			{
				CHIP_ERROR TypeInfo::DecodableType::Decode(TLV::TLVReader &reader,
									   const ConcreteAttributePath &path)
				{
					switch (path.mAttributeId) {
					case Attributes::UTCTime::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, UTCTime);
					case Attributes::Granularity::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, granularity);
					case Attributes::TimeSource::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, timeSource);
					case Attributes::TrustedTimeSource::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, trustedTimeSource);
					case Attributes::DefaultNTP::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, defaultNTP);
					case Attributes::TimeZone::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, timeZone);
					case Attributes::DSTOffset::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, DSTOffset);
					case Attributes::LocalTime::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, localTime);
					case Attributes::TimeZoneDatabase::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, timeZoneDatabase);
					case Attributes::NTPServerAvailable::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, NTPServerAvailable);
					case Attributes::TimeZoneListMaxSize::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, timeZoneListMaxSize);
					case Attributes::DSTOffsetListMaxSize::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, DSTOffsetListMaxSize);
					case Attributes::SupportsDNSResolve::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, supportsDNSResolve);
					case Attributes::GeneratedCommandList::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, generatedCommandList);
					case Attributes::AcceptedCommandList::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, acceptedCommandList);
					case Attributes::AttributeList::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, attributeList);
					case Attributes::FeatureMap::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, featureMap);
					case Attributes::ClusterRevision::TypeInfo::GetAttributeId():
						return DataModel::Decode(reader, clusterRevision);
					default:
						return CHIP_NO_ERROR;
					}
				}
			} // namespace Attributes
		} // namespace TimeSynchronization
	} // namespace Clusters
} // namespace app
} // namespace chip
