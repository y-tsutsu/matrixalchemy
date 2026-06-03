#include "matrixalchemy/scene/CharacterController.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace
{

    float radians(float degrees)
    {
        return degrees * 3.14159265358979323846F / 180.0F;
    }

} // namespace

namespace matrixalchemy::scene
{

    void CharacterController::update(float deltaSeconds, const CharacterInput &input)
    {
        constexpr float moveSpeed = 2.0F;
        constexpr float turnSpeed = 120.0F;

        if (input.turnLeft)
        {
            rotationDegrees_ += turnSpeed * deltaSeconds;
        }
        if (input.turnRight)
        {
            rotationDegrees_ -= turnSpeed * deltaSeconds;
        }

        const float direction = (input.moveForward ? 1.0F : 0.0F) - (input.moveBackward ? 1.0F : 0.0F);
        if (direction != 0.0F)
        {
            const float angle = radians(rotationDegrees_);
            const glm::vec3 forward = {-std::sin(angle), 0.0F, -std::cos(angle)};
            position_ += forward * (direction * moveSpeed * deltaSeconds);
            position_.x = std::clamp(position_.x, -4.5F, 4.5F);
            position_.z = std::clamp(position_.z, -4.5F, 4.5F);
        }

        bounceDegrees_ += 180.0F * deltaSeconds;
        if (bounceDegrees_ >= 360.0F)
        {
            bounceDegrees_ -= 360.0F;
        }
    }

    glm::mat4 CharacterController::transformMatrix() const
    {
        const float bounce = std::sin(radians(bounceDegrees_)) * 0.08F;
        return glm::translate(glm::mat4(1.0F), {position_.x, bounce, position_.z}) * glm::rotate(glm::mat4(1.0F), radians(rotationDegrees_), {0.0F, 1.0F, 0.0F});
    }

} // namespace matrixalchemy::scene
