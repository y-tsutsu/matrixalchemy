#include "GltfModelLoaderInternals.hpp"

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

} // namespace matrixalchemy::asset::gltf
