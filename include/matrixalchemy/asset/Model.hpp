#pragma once

#include "matrixalchemy/asset/ModelData.hpp"
#include "matrixalchemy/asset/ModelPoseAnimator.hpp"
#include "matrixalchemy/render/ModelMesh.hpp"
#include "matrixalchemy/render/ShaderProgram.hpp"
#include "matrixalchemy/render/Texture2D.hpp"

#include <filesystem>
#include <glm/glm.hpp>
#include <optional>
#include <vector>

namespace matrixalchemy::asset
{

    class Model
    {
    public:
        void load(const std::filesystem::path &path, const glm::vec3 &color);
        void release();
        void applyDemoPose(float elapsedSeconds, const PoseAnimationSettings &settings);
        void draw(render::ShaderProgram &shader,
                  const glm::mat4 &modelMatrix,
                  bool useMaterialState = true,
                  const glm::vec3 *toonShadeColor = nullptr,
                  bool useMaterialToonShadeColor = true,
                  bool useMaterialToonLighting = true) const;
        void drawOutline(render::ShaderProgram &shader, const glm::mat4 &modelMatrix, float fallbackWidth, bool useMaterialToonOutline = true) const;

        [[nodiscard]] bool empty() const { return meshes_.empty(); }

    private:
        struct Mesh
        {
            render::ModelMesh geometry;
            std::size_t textureIndex = 0;
            float alphaCutoff = 0.5F;
            ToonMaterial toonMaterial;
            bool hasTexture = false;
            bool alphaMask = false;
            bool alphaBlend = false;
            bool doubleSided = false;
        };

        struct MeshInstance
        {
            std::size_t meshIndex = 0;
            std::size_t nodeIndex = 0;
            std::size_t skinIndex = 0;
            glm::mat4 transform = glm::mat4(1.0F);
            bool hasSkin = false;
        };

        std::vector<Mesh> meshes_;
        std::vector<MeshInstance> instances_;
        std::vector<ModelNode> nodes_;
        std::vector<ModelSkin> skins_;
        std::vector<render::Texture2D> textures_;
        ModelPoseAnimator poseAnimator_;

        [[nodiscard]] std::vector<glm::mat4> jointMatrices(const MeshInstance &instance) const;
        void resetNodeLocalTransforms();
        void updateWorldTransforms();
    };

} // namespace matrixalchemy::asset
