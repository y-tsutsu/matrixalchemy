#include "GltfModelLoaderInternals.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>
#include <utility>

namespace matrixalchemy::asset::gltf
{

    void throwIfFailed(cgltf_result result, const char *message)
    {
        if (result != cgltf_result_success)
        {
            throw std::runtime_error(message);
        }
    }

    glm::mat4 nodeWorldTransform(const cgltf_node &node)
    {
        glm::mat4 transform(1.0F);
        cgltf_node_transform_world(&node, glm::value_ptr(transform));
        return transform;
    }

    namespace
    {

        glm::mat4 nodeLocalTransform(const cgltf_node &node)
        {
            glm::mat4 transform(1.0F);
            cgltf_node_transform_local(&node, glm::value_ptr(transform));
            return transform;
        }

        glm::mat4 readMatrix(const cgltf_accessor *accessor, cgltf_size index)
        {
            glm::mat4 matrix(1.0F);
            if (accessor == nullptr)
            {
                return matrix;
            }

            float values[16] = {};
            cgltf_accessor_read_float(accessor, index, values, 16);
            matrix = glm::make_mat4(values);
            return matrix;
        }

    } // namespace

    std::vector<ModelNode> readNodes(const cgltf_data &data)
    {
        std::vector<ModelNode> nodes;
        nodes.reserve(static_cast<std::size_t>(data.nodes_count));

        for (cgltf_size nodeIndex = 0; nodeIndex < data.nodes_count; ++nodeIndex)
        {
            const cgltf_node &node = data.nodes[nodeIndex];
            ModelNode modelNode;
            modelNode.name = node.name == nullptr ? "" : node.name;
            modelNode.baseLocalTransform = nodeLocalTransform(node);
            modelNode.localTransform = nodeLocalTransform(node);
            modelNode.worldTransform = nodeWorldTransform(node);
            if (node.parent != nullptr)
            {
                modelNode.parentIndex = static_cast<std::size_t>(node.parent - data.nodes);
                modelNode.hasParent = true;
            }
            nodes.push_back(modelNode);
        }

        return nodes;
    }

    std::vector<ModelSkin> readSkins(const cgltf_data &data)
    {
        std::vector<ModelSkin> skins;
        skins.reserve(static_cast<std::size_t>(data.skins_count));

        for (cgltf_size skinIndex = 0; skinIndex < data.skins_count; ++skinIndex)
        {
            const cgltf_skin &skin = data.skins[skinIndex];
            ModelSkin modelSkin;
            modelSkin.jointNodeIndices.reserve(static_cast<std::size_t>(skin.joints_count));
            modelSkin.inverseBindMatrices.reserve(static_cast<std::size_t>(skin.joints_count));

            for (cgltf_size jointIndex = 0; jointIndex < skin.joints_count; ++jointIndex)
            {
                const cgltf_node *jointNode = skin.joints[jointIndex];
                if (jointNode == nullptr)
                {
                    modelSkin.jointNodeIndices.push_back(0);
                }
                else
                {
                    modelSkin.jointNodeIndices.push_back(static_cast<std::size_t>(jointNode - data.nodes));
                }
                modelSkin.inverseBindMatrices.push_back(readMatrix(skin.inverse_bind_matrices, jointIndex));
            }

            skins.push_back(std::move(modelSkin));
        }

        return skins;
    }

} // namespace matrixalchemy::asset::gltf
