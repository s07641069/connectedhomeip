// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DeltadelfinAnimationGradient (cluster code: 293403648/0x117CFC00)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DeltadelfinAnimationGradient/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DeltadelfinAnimationGradient {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace DisplayMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DisplayMode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace DisplayMode
namespace CurrentAnimation {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentAnimation::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace CurrentAnimation
namespace CurrentGradient {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentGradient::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace CurrentGradient
namespace AnimationCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AnimationCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AnimationCount
namespace GradientCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(GradientCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace GradientCount
namespace TransitionTimeMs {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TransitionTimeMs::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace TransitionTimeMs
constexpr std::array<DataModel::AttributeEntry, 5> kMandatoryMetadata = {
    DisplayMode::kMetadataEntry,    CurrentAnimation::kMetadataEntry, CurrentGradient::kMetadataEntry,
    AnimationCount::kMetadataEntry, GradientCount::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace SetAnimation {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetAnimation::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SetAnimation
namespace SetGradient {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetGradient::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SetGradient
namespace SetDisplayMode {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetDisplayMode::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SetDisplayMode

} // namespace Commands

namespace Events {} // namespace Events
} // namespace DeltadelfinAnimationGradient
} // namespace Clusters
} // namespace app
} // namespace chip
