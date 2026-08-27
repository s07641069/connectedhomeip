// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster DeltadelfinAnimationGradient (cluster code: 293403648/0x117CFC00)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DeltadelfinAnimationGradient {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 3;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 0;

namespace SetAnimation {
inline constexpr CommandId Id = 0x00000000;
} // namespace SetAnimation

namespace SetGradient {
inline constexpr CommandId Id = 0x00000001;
} // namespace SetGradient

namespace SetDisplayMode {
inline constexpr CommandId Id = 0x00000002;
} // namespace SetDisplayMode

} // namespace Commands
} // namespace DeltadelfinAnimationGradient
} // namespace Clusters
} // namespace app
} // namespace chip
