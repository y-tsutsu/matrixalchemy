#include "GltfModelLoaderInternals.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <string>
#include <string_view>

namespace matrixalchemy::asset::gltf
{
    namespace
    {

        const cgltf_extension *findExtension(const cgltf_extension *extensions, cgltf_size extensionCount, std::string_view name)
        {
            for (cgltf_size index = 0; index < extensionCount; ++index)
            {
                const cgltf_extension &extension = extensions[index];
                if (extension.name != nullptr && std::string_view(extension.name) == name)
                {
                    return &extension;
                }
            }

            return nullptr;
        }

        std::optional<std::size_t> readVrmHumanoidBoneNode(std::string_view vrmJson, std::string_view boneName)
        {
            std::size_t searchFrom = 0;
            while (true)
            {
                const std::size_t boneKey = vrmJson.find("\"bone\"", searchFrom);
                if (boneKey == std::string_view::npos)
                {
                    return std::nullopt;
                }

                const std::size_t boneValue = vrmJson.find('\"', vrmJson.find(':', boneKey) + 1);
                if (boneValue == std::string_view::npos)
                {
                    return std::nullopt;
                }
                const std::size_t boneValueEnd = vrmJson.find('\"', boneValue + 1);
                if (boneValueEnd == std::string_view::npos)
                {
                    return std::nullopt;
                }

                const std::string_view value = vrmJson.substr(boneValue + 1, boneValueEnd - boneValue - 1);
                if (value != boneName)
                {
                    searchFrom = boneValueEnd + 1;
                    continue;
                }

                const std::size_t objectEnd = vrmJson.find('}', boneValueEnd);
                const std::size_t nodeKey = vrmJson.find("\"node\"", boneValueEnd);
                if (nodeKey == std::string_view::npos || (objectEnd != std::string_view::npos && nodeKey > objectEnd))
                {
                    return std::nullopt;
                }

                const std::size_t nodeColon = vrmJson.find(':', nodeKey);
                if (nodeColon == std::string_view::npos)
                {
                    return std::nullopt;
                }

                std::size_t numberBegin = nodeColon + 1;
                while (numberBegin < vrmJson.size() && std::isspace(static_cast<unsigned char>(vrmJson[numberBegin])) != 0)
                {
                    ++numberBegin;
                }
                std::size_t numberEnd = numberBegin;
                while (numberEnd < vrmJson.size() && std::isdigit(static_cast<unsigned char>(vrmJson[numberEnd])) != 0)
                {
                    ++numberEnd;
                }
                if (numberBegin == numberEnd)
                {
                    return std::nullopt;
                }

                return static_cast<std::size_t>(std::stoul(std::string(vrmJson.substr(numberBegin, numberEnd - numberBegin))));
            }
        }

        std::optional<std::string_view> readJsonStringProperty(std::string_view json, std::string_view propertyName)
        {
            const std::string key = "\"" + std::string(propertyName) + "\"";
            const std::size_t keyPosition = json.find(key);
            if (keyPosition == std::string_view::npos)
            {
                return std::nullopt;
            }

            const std::size_t valueBegin = json.find('\"', json.find(':', keyPosition) + 1);
            if (valueBegin == std::string_view::npos)
            {
                return std::nullopt;
            }
            const std::size_t valueEnd = json.find('\"', valueBegin + 1);
            if (valueEnd == std::string_view::npos)
            {
                return std::nullopt;
            }

            return json.substr(valueBegin + 1, valueEnd - valueBegin - 1);
        }

        std::optional<std::array<float, 4>> readJsonVec4Property(std::string_view json, std::string_view propertyName)
        {
            const std::string key = "\"" + std::string(propertyName) + "\"";
            const std::size_t keyPosition = json.find(key);
            if (keyPosition == std::string_view::npos)
            {
                return std::nullopt;
            }

            const std::size_t arrayBegin = json.find('[', keyPosition);
            const std::size_t arrayEnd = json.find(']', arrayBegin);
            if (arrayBegin == std::string_view::npos || arrayEnd == std::string_view::npos)
            {
                return std::nullopt;
            }

            std::array<float, 4> values = {1.0F, 1.0F, 1.0F, 1.0F};
            std::size_t cursor = arrayBegin + 1;
            for (float &value : values)
            {
                while (cursor < arrayEnd && (std::isspace(static_cast<unsigned char>(json[cursor])) != 0 || json[cursor] == ','))
                {
                    ++cursor;
                }

                const std::size_t valueEnd = json.find_first_of(",]", cursor);
                if (valueEnd == std::string_view::npos || valueEnd > arrayEnd)
                {
                    return std::nullopt;
                }

                value = std::stof(std::string(json.substr(cursor, valueEnd - cursor)));
                cursor = valueEnd + 1;
            }

            return values;
        }

        std::optional<float> readJsonFloatProperty(std::string_view json, std::string_view propertyName)
        {
            const std::string key = "\"" + std::string(propertyName) + "\"";
            const std::size_t keyPosition = json.find(key);
            if (keyPosition == std::string_view::npos)
            {
                return std::nullopt;
            }

            const std::size_t colon = json.find(':', keyPosition);
            if (colon == std::string_view::npos)
            {
                return std::nullopt;
            }

            std::size_t valueBegin = colon + 1;
            while (valueBegin < json.size() && std::isspace(static_cast<unsigned char>(json[valueBegin])) != 0)
            {
                ++valueBegin;
            }

            const std::size_t valueEnd = json.find_first_of(",}", valueBegin);
            if (valueBegin >= json.size() || valueEnd == std::string_view::npos)
            {
                return std::nullopt;
            }

            return std::stof(std::string(json.substr(valueBegin, valueEnd - valueBegin)));
        }

        std::optional<std::string_view> findMaterialPropertyObject(std::string_view vrmJson, std::string_view materialName)
        {
            std::size_t searchFrom = 0;
            while (true)
            {
                const std::size_t nameKey = vrmJson.find("\"name\"", searchFrom);
                if (nameKey == std::string_view::npos)
                {
                    return std::nullopt;
                }

                const std::size_t objectBegin = vrmJson.rfind("{\"floatProperties\"", nameKey);
                const std::size_t windowBegin = objectBegin == std::string_view::npos ? nameKey : objectBegin;
                const std::size_t remainingSize = vrmJson.size() - windowBegin;
                const std::string_view materialWindow = vrmJson.substr(windowBegin, std::min<std::size_t>(remainingSize, 4096));
                const std::optional<std::string_view> name = readJsonStringProperty(materialWindow, "name");
                if (name.has_value() && *name == materialName)
                {
                    return materialWindow;
                }

                searchFrom = nameKey + 1;
            }
        }

        std::optional<std::string_view> findVrmMaterialProperty(const cgltf_material *material, const cgltf_data &data)
        {
            if (material == nullptr || material->name == nullptr)
            {
                return std::nullopt;
            }

            const cgltf_extension *vrmExtension = findExtension(data.data_extensions, data.data_extensions_count, "VRM");
            if (vrmExtension == nullptr || vrmExtension->data == nullptr)
            {
                return std::nullopt;
            }

            return findMaterialPropertyObject(vrmExtension->data, material->name);
        }

        std::optional<glm::vec3> readVrmMaterialVec3Property(const cgltf_material *material, const cgltf_data &data, std::string_view propertyName)
        {
            const std::optional<std::string_view> materialProperty = findVrmMaterialProperty(material, data);
            if (!materialProperty.has_value())
            {
                return std::nullopt;
            }

            const std::optional<std::array<float, 4>> color = readJsonVec4Property(*materialProperty, propertyName);
            if (!color.has_value())
            {
                return std::nullopt;
            }

            return glm::vec3((*color)[0], (*color)[1], (*color)[2]);
        }

        std::optional<glm::vec4> readVrmMaterialVec4Property(const cgltf_material *material, const cgltf_data &data, std::string_view propertyName)
        {
            const std::optional<std::string_view> materialProperty = findVrmMaterialProperty(material, data);
            if (!materialProperty.has_value())
            {
                return std::nullopt;
            }

            const std::optional<std::array<float, 4>> color = readJsonVec4Property(*materialProperty, propertyName);
            if (!color.has_value())
            {
                return std::nullopt;
            }

            return glm::vec4((*color)[0], (*color)[1], (*color)[2], (*color)[3]);
        }

        std::optional<float> readVrmMaterialFloatProperty(const cgltf_material *material, const cgltf_data &data, std::string_view propertyName)
        {
            const std::optional<std::string_view> materialProperty = findVrmMaterialProperty(material, data);
            if (!materialProperty.has_value())
            {
                return std::nullopt;
            }

            return readJsonFloatProperty(*materialProperty, propertyName);
        }

    } // namespace

    void readVrmHumanoid(ModelData &modelData, const cgltf_data &data)
    {
        const cgltf_extension *vrmExtension = findExtension(data.data_extensions, data.data_extensions_count, "VRM");
        if (vrmExtension == nullptr || vrmExtension->data == nullptr)
        {
            return;
        }

        const std::string_view vrmJson(vrmExtension->data);
        modelData.leftUpperArmNodeIndex = readVrmHumanoidBoneNode(vrmJson, "leftUpperArm");
        modelData.rightUpperArmNodeIndex = readVrmHumanoidBoneNode(vrmJson, "rightUpperArm");
        modelData.headNodeIndex = readVrmHumanoidBoneNode(vrmJson, "head");
    }

    ToonMaterial readVrmMaterialToonMaterial(const cgltf_material *material, const cgltf_data &data)
    {
        ToonMaterial toonMaterial;
        toonMaterial.shadeColor = readVrmMaterialVec3Property(material, data, "_ShadeColor");
        const std::optional<glm::vec4> rimColor = readVrmMaterialVec4Property(material, data, "_RimColor");
        if (rimColor.has_value())
        {
            toonMaterial.rimColor = glm::vec3(*rimColor) * rimColor->a;
        }
        toonMaterial.emissionColor = readVrmMaterialVec3Property(material, data, "_EmissionColor");
        toonMaterial.shadeShift = readVrmMaterialFloatProperty(material, data, "_ShadeShift");
        toonMaterial.shadeToony = readVrmMaterialFloatProperty(material, data, "_ShadeToony");
        toonMaterial.rimFresnelPower = readVrmMaterialFloatProperty(material, data, "_RimFresnelPower");
        toonMaterial.outlineColor = readVrmMaterialVec4Property(material, data, "_OutlineColor");
        const std::optional<float> outlineWidth = readVrmMaterialFloatProperty(material, data, "_OutlineWidth");
        if (outlineWidth.has_value())
        {
            toonMaterial.outlineWidth = *outlineWidth * 0.02F;
        }

        return toonMaterial;
    }

} // namespace matrixalchemy::asset::gltf
