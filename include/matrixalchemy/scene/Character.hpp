#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/scene/CharacterController.hpp"
#include "matrixalchemy/scene/IDrawable.hpp"
#include "matrixalchemy/scene/IShadowCaster.hpp"

#include <glm/glm.hpp>

namespace matrixalchemy::scene
{

    class Character final : public IDrawable, public IShadowCaster
    {
    public:
        void create();
        void release();
        void update(float deltaSeconds, const CharacterInput &input);
        void draw(render::ShaderProgram &shader) const override;
        void drawShadow(render::ShaderProgram &shader, const glm::mat4 &shadowMatrix) const override;

        [[nodiscard]] glm::vec3 position() const { return controller_.position(); }
        [[nodiscard]] float rotationDegrees() const { return controller_.rotationDegrees(); }
        [[nodiscard]] float renderHeight() const { return controller_.renderHeight(); }
        [[nodiscard]] glm::mat4 transformMatrix() const { return controller_.transformMatrix(); }

    private:
        render::ColoredMesh mesh_;
        CharacterController controller_;
    };

} // namespace matrixalchemy::scene
