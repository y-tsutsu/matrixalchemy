#pragma once

#include "matrixalchemy/render/ModelMesh.hpp"
#include "matrixalchemy/render/Texture2D.hpp"

#include <glm/glm.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace matrixalchemy::asset
{

    struct ModelPrimitive
    {
        std::vector<render::ModelVertex> vertices;
        std::size_t textureIndex = 0;
        float alphaCutoff = 0.5F;
        bool hasTexture = false;
        bool alphaMask = false;
        bool alphaBlend = false;
        bool doubleSided = false;
    };

    struct ModelInstance
    {
        std::size_t primitiveIndex = 0;
        std::size_t nodeIndex = 0;
        std::size_t skinIndex = 0;
        glm::mat4 transform = glm::mat4(1.0F);
        bool hasSkin = false;
    };

    struct ModelNode
    {
        std::string name;
        std::size_t parentIndex = 0;
        glm::mat4 baseLocalTransform = glm::mat4(1.0F);
        glm::mat4 localTransform = glm::mat4(1.0F);
        glm::mat4 worldTransform = glm::mat4(1.0F);
        bool hasParent = false;
    };

    struct ModelSkin
    {
        std::vector<std::size_t> jointNodeIndices;
        std::vector<glm::mat4> inverseBindMatrices;
    };

    struct ModelData
    {
        std::vector<ModelPrimitive> primitives;
        std::vector<ModelInstance> instances;
        std::vector<ModelNode> nodes;
        std::vector<ModelSkin> skins;
        std::vector<render::Texture2D> textures;
        std::optional<std::size_t> leftUpperArmNodeIndex;
        std::optional<std::size_t> rightUpperArmNodeIndex;
        std::optional<std::size_t> headNodeIndex;
    };

} // namespace matrixalchemy::asset
