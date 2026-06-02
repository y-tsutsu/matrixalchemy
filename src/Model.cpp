#include "matrixalchemy/Model.hpp"

#include "matrixalchemy/Gl.hpp"

#include <cgltf.h>

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

            for (cgltf_size meshIndex = 0; meshIndex < data->meshes_count; ++meshIndex)
            {
                const cgltf_mesh &mesh = data->meshes[meshIndex];
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

                    std::vector<ColoredVertex> vertices;
                    if (primitive.indices != nullptr)
                    {
                        vertices.reserve(static_cast<std::size_t>(primitive.indices->count));
                        for (cgltf_size index = 0; index < primitive.indices->count; ++index)
                        {
                            const cgltf_size vertexIndex = cgltf_accessor_read_index(primitive.indices, index);
                            float position[3] = {};
                            cgltf_accessor_read_float(positions, vertexIndex, position, 3);
                            vertices.push_back({{position[0], position[1], position[2]}, color});
                        }
                    }
                    else
                    {
                        vertices.reserve(static_cast<std::size_t>(positions->count));
                        for (cgltf_size index = 0; index < positions->count; ++index)
                        {
                            float position[3] = {};
                            cgltf_accessor_read_float(positions, index, position, 3);
                            vertices.push_back({{position[0], position[1], position[2]}, color});
                        }
                    }

                    if (!vertices.empty())
                    {
                        ColoredMesh loadedMesh;
                        loadedMesh.upload(vertices, GL_TRIANGLES);
                        meshes_.push_back(std::move(loadedMesh));
                    }
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
        for (ColoredMesh &mesh : meshes_)
        {
            mesh.release();
        }
        meshes_.clear();
    }

    void Model::draw(ShaderProgram &shader, const glm::mat4 &modelMatrix) const
    {
        shader.setMat4("uModel", modelMatrix);
        for (const ColoredMesh &mesh : meshes_)
        {
            mesh.draw();
        }
    }

} // namespace matrixalchemy
