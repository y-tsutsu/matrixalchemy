#include "GltfModelLoaderInternals.hpp"

#include "VrmJsonReader.hpp"

#include <optional>
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

        std::optional<std::string_view> findVrmJson(const cgltf_data &data)
        {
            const cgltf_extension *vrmExtension = findExtension(data.data_extensions, data.data_extensions_count, "VRM");
            if (vrmExtension == nullptr || vrmExtension->data == nullptr)
            {
                return std::nullopt;
            }

            return std::string_view(vrmExtension->data);
        }

    } // namespace

    void readVrmHumanoid(ModelData &modelData, const cgltf_data &data)
    {
        const std::optional<std::string_view> vrmJson = findVrmJson(data);
        if (!vrmJson.has_value())
        {
            return;
        }

        modelData.leftUpperArmNodeIndex = readVrmHumanoidBoneNode(*vrmJson, "leftUpperArm");
        modelData.rightUpperArmNodeIndex = readVrmHumanoidBoneNode(*vrmJson, "rightUpperArm");
        modelData.headNodeIndex = readVrmHumanoidBoneNode(*vrmJson, "head");
    }

    ToonMaterial readVrmMaterialToonMaterial(const cgltf_material *material, const cgltf_data &data)
    {
        if (material == nullptr || material->name == nullptr)
        {
            return {};
        }

        const std::optional<std::string_view> vrmJson = findVrmJson(data);
        if (!vrmJson.has_value())
        {
            return {};
        }

        return readVrmToonMaterial(*vrmJson, material->name);
    }

} // namespace matrixalchemy::asset::gltf
