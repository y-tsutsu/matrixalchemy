#pragma once

#include "matrixalchemy/asset/ModelData.hpp"

#include <cgltf.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <vector>

namespace matrixalchemy::asset::gltf
{

    void throwIfFailed(cgltf_result result, const char *message);
    glm::mat4 nodeWorldTransform(const cgltf_node &node);

    std::vector<ModelPrimitive> readMeshPrimitives(const cgltf_mesh &mesh,
                                                   const cgltf_data &data,
                                                   const std::filesystem::path &modelPath,
                                                   const glm::vec3 &fallbackColor,
                                                   std::vector<std::size_t> &textureIndices,
                                                   std::vector<render::Texture2D> &textures);
    std::vector<ModelNode> readNodes(const cgltf_data &data);
    std::vector<ModelSkin> readSkins(const cgltf_data &data);
    void readVrmHumanoid(ModelData &modelData, const cgltf_data &data);

} // namespace matrixalchemy::asset::gltf
