#include "matrixalchemy/asset/GltfModelLoader.hpp"

#include "GltfModelLoaderInternals.hpp"

#include <cgltf.h>

#include <functional>
#include <limits>
#include <utility>
#include <vector>

namespace matrixalchemy::asset
{

    ModelData loadGltfModel(const std::filesystem::path &path, const glm::vec3 &fallbackColor)
    {
        cgltf_options options{};
        cgltf_data *data = nullptr;

        gltf::throwIfFailed(cgltf_parse_file(&options, path.string().c_str(), &data), "Failed to parse glTF file.");

        try
        {
            gltf::throwIfFailed(cgltf_load_buffers(&options, data, path.string().c_str()), "Failed to load glTF buffers.");
            gltf::throwIfFailed(cgltf_validate(data), "Failed to validate glTF file.");

            ModelData modelData;
            modelData.nodes = gltf::readNodes(*data);
            modelData.skins = gltf::readSkins(*data);
            gltf::readVrmHumanoid(modelData, *data);

            std::vector<std::size_t> textureIndices(data->textures_count, std::numeric_limits<std::size_t>::max());
            std::vector<std::vector<std::size_t>> meshPrimitiveIndices(data->meshes_count);
            for (cgltf_size meshIndex = 0; meshIndex < data->meshes_count; ++meshIndex)
            {
                std::vector<ModelPrimitive> primitives = gltf::readMeshPrimitives(data->meshes[meshIndex], *data, path, fallbackColor, textureIndices, modelData.textures);
                for (ModelPrimitive &primitive : primitives)
                {
                    if (primitive.vertices.empty())
                    {
                        continue;
                    }

                    modelData.primitives.push_back(std::move(primitive));
                    meshPrimitiveIndices[meshIndex].push_back(modelData.primitives.size() - 1);
                }
            }

            const cgltf_scene *scene = data->scene;
            if (scene == nullptr && data->scenes_count > 0)
            {
                scene = &data->scenes[0];
            }

            if (scene != nullptr)
            {
                const std::function<void(const cgltf_node *)> addNodeInstances = [&](const cgltf_node *node)
                {
                    if (node == nullptr)
                    {
                        return;
                    }

                    if (node->mesh != nullptr)
                    {
                        const cgltf_size meshIndex = static_cast<cgltf_size>(node->mesh - data->meshes);
                        const std::size_t nodeIndex = static_cast<std::size_t>(node - data->nodes);
                        if (meshIndex < meshPrimitiveIndices.size())
                        {
                            const bool hasSkin = node->skin != nullptr;
                            const std::size_t skinIndex = hasSkin ? static_cast<std::size_t>(node->skin - data->skins) : 0;
                            for (const std::size_t primitiveIndex : meshPrimitiveIndices[meshIndex])
                            {
                                modelData.instances.push_back({primitiveIndex, nodeIndex, skinIndex, gltf::nodeWorldTransform(*node), hasSkin});
                            }
                        }
                    }

                    for (cgltf_size childIndex = 0; childIndex < node->children_count; ++childIndex)
                    {
                        addNodeInstances(node->children[childIndex]);
                    }
                };

                for (cgltf_size nodeIndex = 0; nodeIndex < scene->nodes_count; ++nodeIndex)
                {
                    addNodeInstances(scene->nodes[nodeIndex]);
                }
            }

            cgltf_free(data);
            return modelData;
        }
        catch (...)
        {
            cgltf_free(data);
            throw;
        }
    }

} // namespace matrixalchemy::asset
