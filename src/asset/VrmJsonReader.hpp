#pragma once

#include "matrixalchemy/asset/ModelData.hpp"

#include <cstddef>
#include <optional>
#include <string_view>

namespace matrixalchemy::asset::gltf
{

    [[nodiscard]] std::optional<std::size_t> readVrmHumanoidBoneNode(std::string_view vrmJson, std::string_view boneName);
    [[nodiscard]] ToonMaterial readVrmToonMaterial(std::string_view vrmJson, std::string_view materialName);

} // namespace matrixalchemy::asset::gltf
