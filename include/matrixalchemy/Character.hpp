#pragma once

#include "matrixalchemy/ColoredMesh.hpp"
#include "matrixalchemy/ShaderProgram.hpp"

#include <glm/glm.hpp>

namespace matrixalchemy
{

    struct CharacterInput
    {
        bool moveForward = false;
        bool moveBackward = false;
        bool turnLeft = false;
        bool turnRight = false;
    };

    class Character
    {
    public:
        void create();
        void release();
        void update(float deltaSeconds, const CharacterInput &input);
        void draw(ShaderProgram &shader) const;

        [[nodiscard]] glm::vec3 position() const { return position_; }
        [[nodiscard]] float rotationDegrees() const { return rotationDegrees_; }

    private:
        ColoredMesh mesh_;
        glm::vec3 position_ = {0.0F, 0.0F, 0.0F};
        float rotationDegrees_ = 0.0F;
        float bounceDegrees_ = 0.0F;
    };

} // namespace matrixalchemy
