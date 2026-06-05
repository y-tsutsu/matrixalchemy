#include "VrmJsonReader.hpp"

#include <cassert>
#include <cmath>
#include <optional>
#include <string_view>

namespace
{

    bool nearlyEqual(float lhs, float rhs)
    {
        return std::fabs(lhs - rhs) < 0.0001F;
    }

    void testHumanoidBoneLookup()
    {
        constexpr std::string_view vrmJson = R"json(
        {
          "humanoid": {
            "humanBones": [
              {"bone":"hips","node":0},
              {"bone":"leftUpperArm","node":15},
              {"bone":"rightUpperArm","node":38},
              {"bone":"head","node":35}
            ]
          }
        }
        )json";

        const std::optional<std::size_t> leftUpperArm = matrixalchemy::asset::gltf::readVrmHumanoidBoneNode(vrmJson, "leftUpperArm");
        const std::optional<std::size_t> missing = matrixalchemy::asset::gltf::readVrmHumanoidBoneNode(vrmJson, "tail");

        assert(leftUpperArm.has_value());
        assert(*leftUpperArm == 15);
        assert(!missing.has_value());
    }

    void testMToonMaterialLookup()
    {
        constexpr std::string_view vrmJson = R"json(
        {
          "materialProperties": [
            {
              "floatProperties": {
                "_OutlineWidth": 0.5,
                "_ShadeShift": -0.1,
                "_ShadeToony": 0.9,
                "_RimFresnelPower": 1.5
              },
              "keywordMap": {},
              "name": "body",
              "shader": "VRM/MToon",
              "vectorProperties": {
                "_ShadeColor": [0.97, 0.81, 0.86, 1],
                "_RimColor": [0.2, 0.3, 0.4, 0.5],
                "_EmissionColor": [0.1, 0.2, 0.3, 1],
                "_OutlineColor": [0.0, 0.0, 0.0, 1]
              }
            }
          ]
        }
        )json";

        const matrixalchemy::asset::ToonMaterial material = matrixalchemy::asset::gltf::readVrmToonMaterial(vrmJson, "body");

        assert(material.shadeColor.has_value());
        assert(nearlyEqual(material.shadeColor->r, 0.97F));
        assert(nearlyEqual(material.shadeColor->g, 0.81F));
        assert(nearlyEqual(material.shadeColor->b, 0.86F));

        assert(material.rimColor.has_value());
        assert(nearlyEqual(material.rimColor->r, 0.1F));
        assert(nearlyEqual(material.rimColor->g, 0.15F));
        assert(nearlyEqual(material.rimColor->b, 0.2F));

        assert(material.emissionColor.has_value());
        assert(nearlyEqual(material.emissionColor->r, 0.1F));
        assert(nearlyEqual(material.emissionColor->g, 0.2F));
        assert(nearlyEqual(material.emissionColor->b, 0.3F));

        assert(material.outlineColor.has_value());
        assert(nearlyEqual(material.outlineColor->a, 1.0F));
        assert(material.outlineWidth.has_value());
        assert(nearlyEqual(*material.outlineWidth, 0.01F));
        assert(material.shadeShift.has_value());
        assert(nearlyEqual(*material.shadeShift, -0.1F));
        assert(material.shadeToony.has_value());
        assert(nearlyEqual(*material.shadeToony, 0.9F));
        assert(material.rimFresnelPower.has_value());
        assert(nearlyEqual(*material.rimFresnelPower, 1.5F));
    }

    void testMissingMToonMaterialReturnsEmptyValues()
    {
        constexpr std::string_view vrmJson = R"json({"materialProperties":[]})json";
        const matrixalchemy::asset::ToonMaterial material = matrixalchemy::asset::gltf::readVrmToonMaterial(vrmJson, "missing");

        assert(!material.shadeColor.has_value());
        assert(!material.rimColor.has_value());
        assert(!material.outlineWidth.has_value());
    }

} // namespace

int main()
{
    testHumanoidBoneLookup();
    testMToonMaterialLookup();
    testMissingMToonMaterialReturnsEmptyValues();
    return 0;
}
