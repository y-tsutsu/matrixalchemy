#pragma once

#include "matrixalchemy/asset/ModelData.hpp"

#include <filesystem>
#include <glm/glm.hpp>

namespace matrixalchemy::asset
{

    ModelData loadGltfModel(const std::filesystem::path &path, const glm::vec3 &fallbackColor);

} // namespace matrixalchemy::asset
