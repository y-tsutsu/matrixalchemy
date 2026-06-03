#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/scene/Drawable.hpp"
#include "matrixalchemy/scene/ShadowCaster.hpp"

#include <glm/glm.hpp>

namespace matrixalchemy::scene
{

    struct CharacterInput
    {
        bool moveForward = false;
        bool moveBackward = false;
        bool turnLeft = false;
        bool turnRight = false;
    };

    class Character final : public Drawable, public ShadowCaster
    {
    public:
        void create();
        void release();
        void update(float deltaSeconds, const CharacterInput &input);
        void draw(render::ShaderProgram &shader) const override;
        void drawShadow(render::ShaderProgram &shader, const glm::mat4 &shadowMatrix) const override;

        [[nodiscard]] glm::vec3 position() const { return position_; }
        [[nodiscard]] float rotationDegrees() const { return rotationDegrees_; }
        [[nodiscard]] glm::mat4 transformMatrix() const;

    private:
        render::ColoredMesh mesh_;
        glm::vec3 position_ = {0.0F, 0.0F, 0.0F};
        float rotationDegrees_ = 0.0F;
        float bounceDegrees_ = 0.0F;
    };

} // namespace matrixalchemy::scene
