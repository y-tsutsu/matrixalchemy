#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/render/ShaderProgram.hpp"

namespace matrixalchemy
{

    class RotatingCube
    {
    public:
        void create(float size);
        void release();
        void update(float deltaSeconds);
        void draw(ShaderProgram &shader) const;
        void drawShadow(ShaderProgram &shader, const glm::mat4 &shadowMatrix) const;

        [[nodiscard]] float rotationDegrees() const { return rotationDegrees_; }

    private:
        [[nodiscard]] glm::mat4 modelMatrix() const;

        ColoredMesh mesh_;
        float rotationDegrees_ = 0.0F;
    };

} // namespace matrixalchemy
