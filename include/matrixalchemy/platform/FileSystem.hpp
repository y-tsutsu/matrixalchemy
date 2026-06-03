#pragma once

#include <filesystem>
#include <string>

namespace matrixalchemy::platform
{

    std::filesystem::path executableDirectory();
    std::filesystem::path resolveAssetPath(const std::filesystem::path &relativePath);
    std::string readTextFile(const std::filesystem::path &path);

} // namespace matrixalchemy::platform
