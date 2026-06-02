#include "matrixalchemy/Model.hpp"

#include "matrixalchemy/Gl.hpp"

#include <cgltf.h>
#include <glm/gtc/type_ptr.hpp>

#include <functional>
#include <stdexcept>

namespace
{

    const cgltf_accessor *findPositionAccessor(const cgltf_primitive &primitive)
    {
        for (cgltf_size index = 0; index < primitive.attributes_count; ++index)
        {
            const cgltf_attribute &attribute = primitive.attributes[index];
            if (attribute.type == cgltf_attribute_type_position)
            {
                return attribute.data;
            }
        }

        return nullptr;
    }

    const cgltf_accessor *findColorAccessor(const cgltf_primitive &primitive)
    {
        for (cgltf_size index = 0; index < primitive.attributes_count; ++index)
        {
            const cgltf_attribute &attribute = primitive.attributes[index];
            if (attribute.type == cgltf_attribute_type_color && attribute.index == 0)
            {
                return attribute.data;
            }
        }

        return nullptr;
    }

    glm::vec3 readVertexColor(const cgltf_accessor *colors, cgltf_size index, const glm::vec3 &fallbackColor)
    {
        if (colors == nullptr)
        {
            return fallbackColor;
        }

        float color[4] = {fallbackColor.r, fallbackColor.g, fallbackColor.b, 1.0F};
        cgltf_accessor_read_float(colors, index, color, 4);
        return {color[0], color[1], color[2]};
    }

    glm::mat4 nodeWorldTransform(const cgltf_node &node)
    {
        glm::mat4 transform(1.0F);
        cgltf_node_transform_world(&node, glm::value_ptr(transform));
        return transform;
    }

    void throwIfFailed(cgltf_result result, const char *message)
    {
        if (result != cgltf_result_success)
        {
            throw std::runtime_error(message);
        }
    }

} // namespace

namespace matrixalchemy
{

    namespace
    {

        std::vector<std::vector<ColoredVertex>> readMeshPrimitives(const cgltf_mesh &mesh, const glm::vec3 &fallbackColor)
        {
            std::vector<std::vector<ColoredVertex>> primitives;

            for (cgltf_size primitiveIndex = 0; primitiveIndex < mesh.primitives_count; ++primitiveIndex)
            {
                const cgltf_primitive &primitive = mesh.primitives[primitiveIndex];
                if (primitive.type != cgltf_primitive_type_triangles)
                {
                    continue;
                }

                const cgltf_accessor *positions = findPositionAccessor(primitive);
                if (positions == nullptr)
                {
                    continue;
                }
                const cgltf_accessor *colors = findColorAccessor(primitive);

                std::vector<ColoredVertex> vertices;
                if (primitive.indices != nullptr)
                {
                    vertices.reserve(static_cast<std::size_t>(primitive.indices->count));
                    for (cgltf_size index = 0; index < primitive.indices->count; ++index)
                    {
                        const cgltf_size vertexIndex = cgltf_accessor_read_index(primitive.indices, index);
                        float position[3] = {};
                        cgltf_accessor_read_float(positions, vertexIndex, position, 3);
                        vertices.push_back({{position[0], position[1], position[2]}, readVertexColor(colors, vertexIndex, fallbackColor)});
                    }
                }
                else
                {
                    vertices.reserve(static_cast<std::size_t>(positions->count));
                    for (cgltf_size index = 0; index < positions->count; ++index)
                    {
                        float position[3] = {};
                        cgltf_accessor_read_float(positions, index, position, 3);
                        vertices.push_back({{position[0], position[1], position[2]}, readVertexColor(colors, index, fallbackColor)});
                    }
                }

                primitives.push_back(std::move(vertices));
            }

            return primitives;
        }

    } // namespace

    void Model::load(const std::filesystem::path &path, const glm::vec3 &color)
    {
        release();

        cgltf_options options{};
        cgltf_data *data = nullptr;

        throwIfFailed(cgltf_parse_file(&options, path.string().c_str(), &data), "Failed to parse glTF file.");

        try
        {
            throwIfFailed(cgltf_load_buffers(&options, data, path.string().c_str()), "Failed to load glTF buffers.");
            throwIfFailed(cgltf_validate(data), "Failed to validate glTF file.");

            std::vector<std::vector<std::size_t>> meshPrimitiveIndices(data->meshes_count);
            for (cgltf_size meshIndex = 0; meshIndex < data->meshes_count; ++meshIndex)
            {
                const std::vector<std::vector<ColoredVertex>> primitives = readMeshPrimitives(data->meshes[meshIndex], color);
                for (const std::vector<ColoredVertex> &vertices : primitives)
                {
                    if (vertices.empty())
                    {
                        continue;
                    }

                    Mesh loadedMesh;
                    loadedMesh.geometry.upload(vertices, GL_TRIANGLES);
                    meshes_.push_back(std::move(loadedMesh));
                    meshPrimitiveIndices[meshIndex].push_back(meshes_.size() - 1);
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
                        if (meshIndex < meshPrimitiveIndices.size())
                        {
                            for (const std::size_t loadedMeshIndex : meshPrimitiveIndices[meshIndex])
                            {
                                instances_.push_back({loadedMeshIndex, nodeWorldTransform(*node)});
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
        }
        catch (...)
        {
            cgltf_free(data);
            release();
            throw;
        }
    }

    void Model::release()
    {
        for (Mesh &mesh : meshes_)
        {
            mesh.geometry.release();
        }
        meshes_.clear();
        instances_.clear();
    }

    void Model::draw(ShaderProgram &shader, const glm::mat4 &modelMatrix) const
    {
        for (const MeshInstance &instance : instances_)
        {
            if (instance.meshIndex >= meshes_.size())
            {
                continue;
            }

            shader.setMat4("uModel", modelMatrix * instance.transform);
            meshes_[instance.meshIndex].geometry.draw();
        }
    }

} // namespace matrixalchemy
