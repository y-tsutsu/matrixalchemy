#include "matrixalchemy/Character.hpp"

#include "matrixalchemy/Gl.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{

    float radians(float degrees)
    {
        return degrees * 3.14159265358979323846F / 180.0F;
    }

    void addBox(std::vector<matrixalchemy::ColoredVertex> &vertices, const glm::vec3 &center, const glm::vec3 &halfSize, const glm::vec3 &color)
    {
        const float left = center.x - halfSize.x;
        const float right = center.x + halfSize.x;
        const float bottom = center.y - halfSize.y;
        const float top = center.y + halfSize.y;
        const float back = center.z - halfSize.z;
        const float front = center.z + halfSize.z;

        const auto push = [&](float x, float y, float z)
        {
            vertices.push_back({{x, y, z}, color});
        };

        push(left, bottom, back);
        push(right, bottom, back);
        push(right, top, back);
        push(right, top, back);
        push(left, top, back);
        push(left, bottom, back);

        push(left, bottom, front);
        push(right, bottom, front);
        push(right, top, front);
        push(right, top, front);
        push(left, top, front);
        push(left, bottom, front);

        push(left, top, front);
        push(left, top, back);
        push(left, bottom, back);
        push(left, bottom, back);
        push(left, bottom, front);
        push(left, top, front);

        push(right, top, front);
        push(right, top, back);
        push(right, bottom, back);
        push(right, bottom, back);
        push(right, bottom, front);
        push(right, top, front);

        push(left, bottom, back);
        push(right, bottom, back);
        push(right, bottom, front);
        push(right, bottom, front);
        push(left, bottom, front);
        push(left, bottom, back);

        push(left, top, back);
        push(right, top, back);
        push(right, top, front);
        push(right, top, front);
        push(left, top, front);
        push(left, top, back);
    }

} // namespace

namespace matrixalchemy
{

    void Character::create()
    {
        constexpr glm::vec3 bodyColor = {0.95F, 0.82F, 0.18F};
        constexpr glm::vec3 faceColor = {1.00F, 0.92F, 0.30F};
        constexpr glm::vec3 eyeColor = {0.06F, 0.05F, 0.04F};
        constexpr glm::vec3 footColor = {0.85F, 0.16F, 0.16F};

        std::vector<ColoredVertex> vertices;
        vertices.reserve(36 * 7);

        addBox(vertices, {0.0F, 0.65F, 0.0F}, {0.35F, 0.45F, 0.25F}, bodyColor);
        addBox(vertices, {0.0F, 1.25F, 0.0F}, {0.30F, 0.25F, 0.25F}, faceColor);
        addBox(vertices, {-0.12F, 1.30F, -0.26F}, {0.04F, 0.04F, 0.02F}, eyeColor);
        addBox(vertices, {0.12F, 1.30F, -0.26F}, {0.04F, 0.04F, 0.02F}, eyeColor);
        addBox(vertices, {-0.20F, 0.10F, -0.05F}, {0.13F, 0.10F, 0.22F}, footColor);
        addBox(vertices, {0.20F, 0.10F, -0.05F}, {0.13F, 0.10F, 0.22F}, footColor);

        mesh_.upload(vertices, GL_TRIANGLES);
    }

    void Character::release()
    {
        mesh_.release();
    }

    void Character::update(float deltaSeconds, const CharacterInput &input)
    {
        constexpr float moveSpeed = 2.0F;
        constexpr float turnSpeed = 120.0F;

        if (input.turnLeft)
        {
            rotationDegrees_ -= turnSpeed * deltaSeconds;
        }
        if (input.turnRight)
        {
            rotationDegrees_ += turnSpeed * deltaSeconds;
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

    void Character::draw(ShaderProgram &shader) const
    {
        shader.setMat4("uModel", modelMatrix());
        mesh_.draw();
    }

    void Character::drawShadow(ShaderProgram &shader, const glm::mat4 &shadowMatrix) const
    {
        const glm::mat4 floorLift = glm::translate(glm::mat4(1.0F), {0.0F, 0.015F, 0.0F});
        shader.setMat4("uModel", floorLift * shadowMatrix * modelMatrix());
        mesh_.draw();
    }

    glm::mat4 Character::modelMatrix() const
    {
        const float bounce = std::sin(radians(bounceDegrees_)) * 0.08F;
        return glm::translate(glm::mat4(1.0F), {position_.x, bounce, position_.z}) * glm::rotate(glm::mat4(1.0F), radians(rotationDegrees_), {0.0F, 1.0F, 0.0F});
    }

} // namespace matrixalchemy
