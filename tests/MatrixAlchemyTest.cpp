#include "GltfModelLoaderInternals.hpp"
#include "VrmJsonReader.hpp"
#include "matrixalchemy/render/Shadow.hpp"
#include "matrixalchemy/scene/CharacterController.hpp"
#include "matrixalchemy/scene/OrbitCamera.hpp"

#include <cassert>
#include <cgltf.h>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <string_view>

namespace
{

    bool nearlyEqual(float lhs, float rhs)
    {
        return std::fabs(lhs - rhs) < 0.0001F;
    }

    bool nearlyEqual(const glm::vec3 &lhs, const glm::vec3 &rhs)
    {
        return nearlyEqual(lhs.x, rhs.x) && nearlyEqual(lhs.y, rhs.y) && nearlyEqual(lhs.z, rhs.z);
    }

    glm::vec3 projectedPoint(const glm::mat4 &matrix, const glm::vec3 &point)
    {
        const glm::vec4 projected = matrix * glm::vec4(point, 1.0F);
        return glm::vec3(projected) / projected.w;
    }

    struct ParsedGltf
    {
        cgltf_data *data = nullptr;

        explicit ParsedGltf(std::string_view json)
        {
            cgltf_options options{};
            const cgltf_result result = cgltf_parse(&options, json.data(), json.size(), &data);
            assert(result == cgltf_result_success);
            assert(data != nullptr);
        }

        ~ParsedGltf()
        {
            if (data != nullptr)
            {
                cgltf_free(data);
            }
        }

        ParsedGltf(const ParsedGltf &) = delete;
        ParsedGltf &operator=(const ParsedGltf &) = delete;
    };

    void testPlanarShadowProjectsPointsOntoPlane()
    {
        const glm::mat4 shadowMatrix = matrixalchemy::render::planarShadowMatrix({0.0F, 1.0F, 0.0F, 0.0F}, {-4.0F, 6.0F, 2.0F, 1.0F});

        const glm::vec3 point = {1.5F, 2.0F, -0.75F};
        const glm::vec3 shadow = projectedPoint(shadowMatrix, point);

        assert(nearlyEqual(shadow.y, 0.0F));
    }

    void testPlanarShadowRejectsParallelLight()
    {
        bool threw = false;
        try
        {
            (void)matrixalchemy::render::planarShadowMatrix({0.0F, 1.0F, 0.0F, 0.0F}, {1.0F, 0.0F, 0.0F, 0.0F});
        }
        catch (const std::runtime_error &)
        {
            threw = true;
        }

        assert(threw);
    }

    void testOrbitCameraInitialPosition()
    {
        const matrixalchemy::scene::OrbitCamera camera;
        const glm::vec3 position = camera.position();

        assert(nearlyEqual(glm::length(position), camera.radius()));
        assert(nearlyEqual(position.y, std::sin(glm::radians(camera.phiDegrees())) * camera.radius()));
    }

    void testOrbitCameraDragAndZoomClamp()
    {
        matrixalchemy::scene::OrbitCamera camera;

        camera.beginDrag(0.0, 0.0);
        camera.drag(10.0, 20.0);
        camera.endDrag();

        assert(nearlyEqual(camera.thetaDegrees(), -39.0F));
        assert(nearlyEqual(camera.phiDegrees(), 33.0F));

        camera.zoom(100.0);
        assert(nearlyEqual(camera.radius(), 3.0F));
        camera.zoom(-100.0);
        assert(nearlyEqual(camera.radius(), 18.0F));
    }

    void testCharacterControllerMovesForwardAndTurns()
    {
        matrixalchemy::scene::CharacterController controller;

        matrixalchemy::scene::CharacterInput input;
        input.moveForward = true;
        controller.update(1.0F, input);

        assert(nearlyEqual(controller.position(), {0.0F, 0.0F, -2.0F}));

        input = {};
        input.turnLeft = true;
        controller.update(0.5F, input);
        assert(nearlyEqual(controller.rotationDegrees(), 60.0F));

        input = {};
        input.moveForward = true;
        controller.update(1.0F, input);
        assert(controller.position().x < -1.7F);
        assert(controller.position().z < -2.9F);
    }

    void testCharacterControllerBoundsPosition()
    {
        matrixalchemy::scene::CharacterController controller;
        matrixalchemy::scene::CharacterInput input;
        input.moveForward = true;

        for (int step = 0; step < 10; ++step)
        {
            controller.update(1.0F, input);
        }

        assert(nearlyEqual(controller.position().z, -4.5F));
    }

    void testGltfNodeReaderKeepsHierarchyAndTransforms()
    {
        constexpr std::string_view gltfJson = R"json(
        {
          "asset": {"version": "2.0"},
          "nodes": [
            {"name": "root", "translation": [1.0, 0.0, 0.0], "children": [1]},
            {"name": "child", "translation": [2.0, 0.0, 0.0]}
          ]
        }
        )json";
        const ParsedGltf gltf(gltfJson);

        const std::vector<matrixalchemy::asset::ModelNode> nodes = matrixalchemy::asset::gltf::readNodes(*gltf.data);

        assert(nodes.size() == 2);
        assert(nodes[0].name == "root");
        assert(!nodes[0].hasParent);
        assert(nodes[1].name == "child");
        assert(nodes[1].hasParent);
        assert(nodes[1].parentIndex == 0);
        assert(nearlyEqual(nodes[0].localTransform[3].x, 1.0F));
        assert(nearlyEqual(nodes[1].localTransform[3].x, 2.0F));
        assert(nearlyEqual(nodes[1].worldTransform[3].x, 3.0F));
    }

    void testGltfSkinReaderUsesJointNodeIndices()
    {
        constexpr std::string_view gltfJson = R"json(
        {
          "asset": {"version": "2.0"},
          "nodes": [{}, {}],
          "skins": [
            {"joints": [1, 0]}
          ]
        }
        )json";
        const ParsedGltf gltf(gltfJson);

        const std::vector<matrixalchemy::asset::ModelSkin> skins = matrixalchemy::asset::gltf::readSkins(*gltf.data);

        assert(skins.size() == 1);
        assert(skins[0].jointNodeIndices.size() == 2);
        assert(skins[0].jointNodeIndices[0] == 1);
        assert(skins[0].jointNodeIndices[1] == 0);
        assert(skins[0].inverseBindMatrices.size() == 2);
        assert(nearlyEqual(skins[0].inverseBindMatrices[0][0].x, 1.0F));
    }

    void testVrmHumanoidBoneLookup()
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

    void testVrmMToonMaterialLookup()
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

    void testVrmMToonMaterialSelectsNamedMaterial()
    {
        constexpr std::string_view vrmJson = R"json(
        {
          "materialProperties": [
            {
              "floatProperties": {"_OutlineWidth": 0.25},
              "name": "first",
              "vectorProperties": {"_ShadeColor": [1, 0, 0, 1]}
            },
            {
              "floatProperties": {"_OutlineWidth": 0.75},
              "name": "second",
              "vectorProperties": {"_ShadeColor": [0, 1, 0, 1]}
            }
          ]
        }
        )json";

        const matrixalchemy::asset::ToonMaterial material = matrixalchemy::asset::gltf::readVrmToonMaterial(vrmJson, "second");

        assert(material.shadeColor.has_value());
        assert(nearlyEqual(material.shadeColor->r, 0.0F));
        assert(nearlyEqual(material.shadeColor->g, 1.0F));
        assert(material.outlineWidth.has_value());
        assert(nearlyEqual(*material.outlineWidth, 0.015F));
    }

    void testMissingVrmMToonMaterialReturnsEmptyValues()
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
    testPlanarShadowProjectsPointsOntoPlane();
    testPlanarShadowRejectsParallelLight();
    testOrbitCameraInitialPosition();
    testOrbitCameraDragAndZoomClamp();
    testCharacterControllerMovesForwardAndTurns();
    testCharacterControllerBoundsPosition();
    testGltfNodeReaderKeepsHierarchyAndTransforms();
    testGltfSkinReaderUsesJointNodeIndices();
    testVrmHumanoidBoneLookup();
    testVrmMToonMaterialLookup();
    testVrmMToonMaterialSelectsNamedMaterial();
    testMissingVrmMToonMaterialReturnsEmptyValues();
    return 0;
}
