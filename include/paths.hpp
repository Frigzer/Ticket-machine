#pragma once

#include <filesystem>

namespace paths {

inline const std::filesystem::path sourceDir = std::filesystem::path( TICKET_MACHINE_SOURCE_DIR );
inline const std::filesystem::path dataDir   = sourceDir / "data";

}  // namespace paths
