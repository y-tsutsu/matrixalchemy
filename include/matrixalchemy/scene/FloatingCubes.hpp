#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/scene/IDrawable.hpp"
#include "matrixalchemy/scene/IShadowCaster.hpp"

#include <random>
#include <vector>

namespace matrixalchemy::scene
{

    class FloatingCubes final : public IDrawable, public IShadowCaster
    {
    public:
        void create(float size);
        void release();
        void update(float deltaSeconds);
        void draw(render::ShaderProgram &shader) const override;
        void drawShadow(render::ShaderProgram &shader, const glm::mat4 &shadowMatrix) const override;

        [[nodiscard]] float rotationDegrees() const;

    private:
        struct CubeInstance
        {
            glm::vec3 basePosition;
            glm::vec3 targetPosition;
            glm::vec3 colorScale;
            glm::vec3 rotationAxis;
            float size = 1.0F;
            float driftSpeed = 0.0F;
            float orbitRadius = 0.0F;
            float orbitSpeed = 0.0F;
            float bobHeight = 0.0F;
            float bobSpeed = 0.0F;
            float phase = 0.0F;
        };

        [[nodiscard]] glm::vec3 randomTargetPosition(float cubeSize);
        [[nodiscard]] glm::mat4 modelMatrix(const CubeInstance &cube) const;

        render::ColoredMesh mesh_;
        std::vector<CubeInstance> cubes_;
        std::mt19937 random_;
        float rotationDegrees_ = 0.0F;
        float elapsedSeconds_ = 0.0F;
    };

} // namespace matrixalchemy::scene
