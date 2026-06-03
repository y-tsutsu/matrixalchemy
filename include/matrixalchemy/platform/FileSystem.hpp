#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace matrixalchemy::platform
{

    std::filesystem::path executableDirectory();
    std::optional<std::filesystem::path> findRuntimeAssetPath(const std::filesystem::path &relativePath);
    std::filesystem::path resolveAssetPath(const std::filesystem::path &relativePath);
    std::string readTextFile(const std::filesystem::path &path);

} // namespace matrixalchemy::platform
