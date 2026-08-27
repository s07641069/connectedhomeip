// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DeltadelfinAnimationGradient (cluster code: 293403648/0x117CFC00)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/DeltadelfinAnimationGradient/Ids.h>
#include <clusters/DeltadelfinAnimationGradient/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::DeltadelfinAnimationGradient::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::DeltadelfinAnimationGradient::Attributes;
        switch (attributeId)
        {
        case DisplayMode::Id:
            return DisplayMode::kMetadataEntry;
        case CurrentAnimation::Id:
            return CurrentAnimation::kMetadataEntry;
        case CurrentGradient::Id:
            return CurrentGradient::kMetadataEntry;
        case AnimationCount::Id:
            return AnimationCount::kMetadataEntry;
        case GradientCount::Id:
            return GradientCount::kMetadataEntry;
        case TransitionTimeMs::Id:
            return TransitionTimeMs::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::DeltadelfinAnimationGradient::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::DeltadelfinAnimationGradient::Commands;
        switch (commandId)
        {
        case SetAnimation::Id:
            return SetAnimation::kMetadataEntry;
        case SetGradient::Id:
            return SetGradient::kMetadataEntry;
        case SetDisplayMode::Id:
            return SetDisplayMode::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
