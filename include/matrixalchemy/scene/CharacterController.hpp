#pragma once

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

    class CharacterController
    {
    public:
        void update(float deltaSeconds, const CharacterInput &input);

        [[nodiscard]] glm::vec3 position() const { return position_; }
        [[nodiscard]] float rotationDegrees() const { return rotationDegrees_; }
        [[nodiscard]] float renderHeight() const;
        [[nodiscard]] glm::mat4 transformMatrix() const;

    private:
        glm::vec3 position_ = {0.0F, 0.0F, 0.0F};
        float rotationDegrees_ = 0.0F;
        float bounceDegrees_ = 0.0F;
    };

} // namespace matrixalchemy::scene
