// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster RefrigeratorAlarm (cluster code: 87/0x57)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip
{
namespace app
{
	namespace Clusters
	{
		namespace RefrigeratorAlarm
		{
			namespace Attributes
			{
				namespace Mask
				{
					inline constexpr AttributeId Id = 0x00000000;
				} // namespace Mask

				namespace State
				{
					inline constexpr AttributeId Id = 0x00000002;
				} // namespace State

				namespace Supported
				{
					inline constexpr AttributeId Id = 0x00000003;
				} // namespace Supported

				namespace GeneratedCommandList
				{
					inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
				} // namespace GeneratedCommandList

				namespace AcceptedCommandList
				{
					inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
				} // namespace AcceptedCommandList

				namespace AttributeList
				{
					inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
				} // namespace AttributeList

				namespace FeatureMap
				{
					inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
				} // namespace FeatureMap

				namespace ClusterRevision
				{
					inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
				} // namespace ClusterRevision

			} // namespace Attributes
		} // namespace RefrigeratorAlarm
	} // namespace Clusters
} // namespace app
} // namespace chip
