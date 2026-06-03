#pragma once

#include "matrixalchemy/render/ModelMesh.hpp"
#include "matrixalchemy/render/ShaderProgram.hpp"
#include "matrixalchemy/render/Texture2D.hpp"

#include <filesystem>
#include <glm/glm.hpp>
#include <vector>

namespace matrixalchemy::asset
{

    class Model
    {
    public:
        void load(const std::filesystem::path &path, const glm::vec3 &color);
        void release();
        void draw(render::ShaderProgram &shader, const glm::mat4 &modelMatrix, bool useMaterialState = true) const;
        void drawOutline(render::ShaderProgram &shader, const glm::mat4 &modelMatrix, float width) const;

        [[nodiscard]] bool empty() const { return meshes_.empty(); }

    private:
        struct Mesh
        {
            render::ModelMesh geometry;
            std::size_t textureIndex = 0;
            float alphaCutoff = 0.5F;
            bool hasTexture = false;
            bool alphaMask = false;
            bool alphaBlend = false;
            bool doubleSided = false;
        };

        struct MeshInstance
        {
            std::size_t meshIndex = 0;
            glm::mat4 transform = glm::mat4(1.0F);
        };

        std::vector<Mesh> meshes_;
        std::vector<MeshInstance> instances_;
        std::vector<render::Texture2D> textures_;
    };

} // namespace matrixalchemy::asset
