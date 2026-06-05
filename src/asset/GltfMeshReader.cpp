#include "GltfModelLoaderInternals.hpp"

#include "matrixalchemy/platform/Gl.hpp"

#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace matrixalchemy::asset::gltf
{
    namespace
    {

        const cgltf_accessor *findAttributeAccessor(const cgltf_primitive &primitive, cgltf_attribute_type type, cgltf_int attributeIndex = 0)
        {
            for (cgltf_size index = 0; index < primitive.attributes_count; ++index)
            {
                const cgltf_attribute &attribute = primitive.attributes[index];
                if (attribute.type == type && attribute.index == attributeIndex)
                {
                    return attribute.data;
                }
            }

            return nullptr;
        }

        glm::vec4 readVertexColor(const cgltf_accessor *colors, cgltf_size index, const glm::vec4 &fallbackColor)
        {
            if (colors == nullptr)
            {
                return fallbackColor;
            }

            float color[4] = {fallbackColor.r, fallbackColor.g, fallbackColor.b, fallbackColor.a};
            cgltf_accessor_read_float(colors, index, color, 4);
            return {color[0], color[1], color[2], color[3]};
        }

        glm::vec2 readTexCoord(const cgltf_accessor *texCoords, cgltf_size index)
        {
            if (texCoords == nullptr)
            {
                return {0.0F, 0.0F};
            }

            float texCoord[2] = {};
            cgltf_accessor_read_float(texCoords, index, texCoord, 2);
            return {texCoord[0], texCoord[1]};
        }

        glm::vec3 readNormal(const cgltf_accessor *normals, cgltf_size index)
        {
            if (normals == nullptr)
            {
                return {0.0F, 1.0F, 0.0F};
            }

            float normal[3] = {0.0F, 1.0F, 0.0F};
            cgltf_accessor_read_float(normals, index, normal, 3);
            return glm::normalize(glm::vec3(normal[0], normal[1], normal[2]));
        }

        glm::uvec4 readJoints(const cgltf_accessor *joints, cgltf_size index)
        {
            if (joints == nullptr)
            {
                return {0, 0, 0, 0};
            }

            float jointValues[4] = {};
            cgltf_accessor_read_float(joints, index, jointValues, 4);
            return {
                static_cast<unsigned int>(jointValues[0]),
                static_cast<unsigned int>(jointValues[1]),
                static_cast<unsigned int>(jointValues[2]),
                static_cast<unsigned int>(jointValues[3]),
            };
        }

        glm::vec4 readWeights(const cgltf_accessor *weights, cgltf_size index)
        {
            if (weights == nullptr)
            {
                return {0.0F, 0.0F, 0.0F, 0.0F};
            }

            float weightValues[4] = {};
            cgltf_accessor_read_float(weights, index, weightValues, 4);
            return {weightValues[0], weightValues[1], weightValues[2], weightValues[3]};
        }

        glm::vec2 applyTextureTransform(const glm::vec2 &texCoord, const cgltf_texture_view &textureView)
        {
            if (!textureView.has_transform)
            {
                return texCoord;
            }

            const glm::vec2 scale(textureView.transform.scale[0], textureView.transform.scale[1]);
            const glm::vec2 offset(textureView.transform.offset[0], textureView.transform.offset[1]);
            return texCoord * scale + offset;
        }

        glm::vec4 materialBaseColor(const cgltf_material *material)
        {
            if (material == nullptr || !material->has_pbr_metallic_roughness)
            {
                return {1.0F, 1.0F, 1.0F, 1.0F};
            }

            const cgltf_pbr_metallic_roughness &pbr = material->pbr_metallic_roughness;
            return {pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2], pbr.base_color_factor[3]};
        }

        const cgltf_texture_view *baseColorTextureView(const cgltf_material *material)
        {
            if (material == nullptr || !material->has_pbr_metallic_roughness)
            {
                return nullptr;
            }

            const cgltf_texture_view &textureView = material->pbr_metallic_roughness.base_color_texture;
            return textureView.texture == nullptr ? nullptr : &textureView;
        }

        unsigned int textureMinFilter(cgltf_filter_type filter)
        {
            if (filter == cgltf_filter_type_nearest || filter == cgltf_filter_type_linear || filter == cgltf_filter_type_nearest_mipmap_nearest ||
                filter == cgltf_filter_type_linear_mipmap_nearest || filter == cgltf_filter_type_nearest_mipmap_linear || filter == cgltf_filter_type_linear_mipmap_linear)
            {
                return static_cast<unsigned int>(filter);
            }

            return GL_LINEAR_MIPMAP_LINEAR;
        }

        unsigned int textureMagFilter(cgltf_filter_type filter)
        {
            if (filter == cgltf_filter_type_nearest || filter == cgltf_filter_type_linear)
            {
                return static_cast<unsigned int>(filter);
            }

            return GL_LINEAR;
        }

        unsigned int textureWrap(cgltf_wrap_mode wrap)
        {
            if (wrap == cgltf_wrap_mode_clamp_to_edge || wrap == cgltf_wrap_mode_mirrored_repeat || wrap == cgltf_wrap_mode_repeat)
            {
                return static_cast<unsigned int>(wrap);
            }

            return GL_REPEAT;
        }

        render::TextureSampling textureSampling(const cgltf_texture &texture)
        {
            if (texture.sampler == nullptr)
            {
                return {GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT};
            }

            return {textureMinFilter(texture.sampler->min_filter), textureMagFilter(texture.sampler->mag_filter), textureWrap(texture.sampler->wrap_s), textureWrap(texture.sampler->wrap_t)};
        }

        std::vector<unsigned char> readBinaryFile(const std::filesystem::path &path)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
            {
                throw std::runtime_error("Failed to open texture image file.");
            }

            return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        }

        std::optional<std::size_t> loadTexture(const cgltf_texture &texture,
                                               const cgltf_data &data,
                                               const std::filesystem::path &modelPath,
                                               std::vector<std::size_t> &textureIndices,
                                               std::vector<render::Texture2D> &textures)
        {
            const cgltf_size cgltfTextureIndex = static_cast<cgltf_size>(&texture - data.textures);
            if (cgltfTextureIndex >= textureIndices.size())
            {
                return std::nullopt;
            }

            const std::size_t unloaded = std::numeric_limits<std::size_t>::max();
            if (textureIndices[cgltfTextureIndex] != unloaded)
            {
                return textureIndices[cgltfTextureIndex];
            }

            const cgltf_image *image = texture.image;
            if (image == nullptr)
            {
                return std::nullopt;
            }

            render::Texture2D loadedTexture;
            const render::TextureSampling sampling = textureSampling(texture);
            if (image->buffer_view != nullptr)
            {
                const unsigned char *imageData = cgltf_buffer_view_data(image->buffer_view);
                loadedTexture.loadFromMemory({imageData, image->buffer_view->size}, sampling);
            }
            else if (image->uri != nullptr)
            {
                const std::string uri(image->uri);
                if (uri.starts_with("data:"))
                {
                    return std::nullopt;
                }

                const std::vector<unsigned char> imageData = readBinaryFile(modelPath.parent_path() / uri);
                loadedTexture.loadFromMemory(imageData, sampling);
            }
            else
            {
                return std::nullopt;
            }

            textures.push_back(std::move(loadedTexture));
            textureIndices[cgltfTextureIndex] = textures.size() - 1;
            return textureIndices[cgltfTextureIndex];
        }

        render::ModelVertex readVertex(const cgltf_accessor &positions,
                                       cgltf_size vertexIndex,
                                       const cgltf_accessor *colors,
                                       const cgltf_accessor *texCoords,
                                       const cgltf_accessor *normals,
                                       const cgltf_accessor *joints,
                                       const cgltf_accessor *weights,
                                       const cgltf_texture_view *textureView,
                                       const glm::vec4 &fallbackVertexColor,
                                       const glm::vec4 &baseColor)
        {
            float position[3] = {};
            cgltf_accessor_read_float(&positions, vertexIndex, position, 3);
            const glm::vec2 rawTexCoord = readTexCoord(texCoords, vertexIndex);

            return {{position[0], position[1], position[2]},
                    readVertexColor(colors, vertexIndex, fallbackVertexColor) * baseColor,
                    textureView == nullptr ? rawTexCoord : applyTextureTransform(rawTexCoord, *textureView),
                    readNormal(normals, vertexIndex),
                    readJoints(joints, vertexIndex),
                    readWeights(weights, vertexIndex)};
        }

    } // namespace

    std::vector<ModelPrimitive> readMeshPrimitives(const cgltf_mesh &mesh,
                                                   const cgltf_data &data,
                                                   const std::filesystem::path &modelPath,
                                                   const glm::vec3 &fallbackColor,
                                                   std::vector<std::size_t> &textureIndices,
                                                   std::vector<render::Texture2D> &textures)
    {
        std::vector<ModelPrimitive> primitives;

        for (cgltf_size primitiveIndex = 0; primitiveIndex < mesh.primitives_count; ++primitiveIndex)
        {
            const cgltf_primitive &primitive = mesh.primitives[primitiveIndex];
            if (primitive.type != cgltf_primitive_type_triangles)
            {
                continue;
            }

            const cgltf_accessor *positions = findAttributeAccessor(primitive, cgltf_attribute_type_position);
            if (positions == nullptr)
            {
                continue;
            }
            const cgltf_accessor *colors = findAttributeAccessor(primitive, cgltf_attribute_type_color);
            const cgltf_accessor *texCoords = findAttributeAccessor(primitive, cgltf_attribute_type_texcoord);
            const cgltf_accessor *normals = findAttributeAccessor(primitive, cgltf_attribute_type_normal);
            const cgltf_accessor *joints = findAttributeAccessor(primitive, cgltf_attribute_type_joints);
            const cgltf_accessor *weights = findAttributeAccessor(primitive, cgltf_attribute_type_weights);
            const glm::vec4 baseColor = materialBaseColor(primitive.material);
            const cgltf_texture_view *textureView = baseColorTextureView(primitive.material);
            const std::optional<std::size_t> textureIndex =
                textureView != nullptr && texCoords != nullptr ? loadTexture(*textureView->texture, data, modelPath, textureIndices, textures) : std::nullopt;
            const glm::vec4 fallbackVertexColor = textureIndex.has_value() ? glm::vec4(1.0F) : glm::vec4(fallbackColor, 1.0F);

            ModelPrimitive loadedPrimitive;
            loadedPrimitive.textureIndex = textureIndex.value_or(0);
            loadedPrimitive.hasTexture = textureIndex.has_value();
            loadedPrimitive.alphaCutoff = primitive.material == nullptr ? 0.5F : primitive.material->alpha_cutoff;
            loadedPrimitive.toonShadeColor = readVrmMaterialShadeColor(primitive.material, data);
            loadedPrimitive.toonRimColor = readVrmMaterialRimColor(primitive.material, data);
            loadedPrimitive.toonEmissionColor = readVrmMaterialEmissionColor(primitive.material, data);
            loadedPrimitive.toonOutlineColor = readVrmMaterialOutlineColor(primitive.material, data);
            loadedPrimitive.toonOutlineWidth = readVrmMaterialOutlineWidth(primitive.material, data);
            loadedPrimitive.alphaMask = primitive.material != nullptr && primitive.material->alpha_mode == cgltf_alpha_mode_mask;
            loadedPrimitive.alphaBlend = primitive.material != nullptr && primitive.material->alpha_mode == cgltf_alpha_mode_blend;
            loadedPrimitive.doubleSided = primitive.material != nullptr && primitive.material->double_sided;

            if (primitive.indices != nullptr)
            {
                loadedPrimitive.vertices.reserve(static_cast<std::size_t>(primitive.indices->count));
                for (cgltf_size index = 0; index < primitive.indices->count; ++index)
                {
                    const cgltf_size vertexIndex = cgltf_accessor_read_index(primitive.indices, index);
                    loadedPrimitive.vertices.push_back(readVertex(*positions, vertexIndex, colors, texCoords, normals, joints, weights, textureView, fallbackVertexColor, baseColor));
                }
            }
            else
            {
                loadedPrimitive.vertices.reserve(static_cast<std::size_t>(positions->count));
                for (cgltf_size index = 0; index < positions->count; ++index)
                {
                    loadedPrimitive.vertices.push_back(readVertex(*positions, index, colors, texCoords, normals, joints, weights, textureView, fallbackVertexColor, baseColor));
                }
            }

            primitives.push_back(std::move(loadedPrimitive));
        }

        return primitives;
    }

} // namespace matrixalchemy::asset::gltf
