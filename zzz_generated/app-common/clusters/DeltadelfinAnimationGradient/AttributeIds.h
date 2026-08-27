// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster DeltadelfinAnimationGradient (cluster code: 293403648/0x117CFC00)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DeltadelfinAnimationGradient {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 11;

namespace DisplayMode {
inline constexpr AttributeId Id = 0x00000000;
} // namespace DisplayMode

namespace CurrentAnimation {
inline constexpr AttributeId Id = 0x00000001;
} // namespace CurrentAnimation

namespace CurrentGradient {
inline constexpr AttributeId Id = 0x00000002;
} // namespace CurrentGradient

namespace AnimationCount {
inline constexpr AttributeId Id = 0x00000003;
} // namespace AnimationCount

namespace GradientCount {
inline constexpr AttributeId Id = 0x00000004;
} // namespace GradientCount

namespace TransitionTimeMs {
inline constexpr AttributeId Id = 0x00000005;
} // namespace TransitionTimeMs

namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

namespace AttributeList {
inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList

namespace FeatureMap {
inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap

namespace ClusterRevision {
inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes
} // namespace DeltadelfinAnimationGradient
} // namespace Clusters
} // namespace app
} // namespace chip
