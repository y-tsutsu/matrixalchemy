#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/scene/IDrawable.hpp"
#include "matrixalchemy/scene/IShadowCaster.hpp"

namespace matrixalchemy::scene
{

    class RotatingCube final : public IDrawable, public IShadowCaster
    {
    public:
        void create(float size);
        void release();
        void update(float deltaSeconds);
        void draw(render::ShaderProgram &shader) const override;
        void drawShadow(render::ShaderProgram &shader, const glm::mat4 &shadowMatrix) const override;

        [[nodiscard]] float rotationDegrees() const { return rotationDegrees_; }

    private:
        [[nodiscard]] glm::mat4 modelMatrix() const;

        render::ColoredMesh mesh_;
        float rotationDegrees_ = 0.0F;
    };

} // namespace matrixalchemy::scene
