#include "matrixalchemy/scene/Character.hpp"

#include "matrixalchemy/platform/Gl.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace
{

    void addBox(std::vector<matrixalchemy::render::ColoredVertex> &vertices, const glm::vec3 &center, const glm::vec3 &halfSize, const glm::vec3 &color)
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

namespace matrixalchemy::scene
{

    void Character::create()
    {
        constexpr glm::vec3 bodyColor = {0.95F, 0.82F, 0.18F};
        constexpr glm::vec3 faceColor = {1.00F, 0.92F, 0.30F};
        constexpr glm::vec3 eyeColor = {0.06F, 0.05F, 0.04F};
        constexpr glm::vec3 footColor = {0.85F, 0.16F, 0.16F};

        std::vector<render::ColoredVertex> vertices;
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

    void Character::updateWithInput(float deltaSeconds, const CharacterInput &input)
    {
        controller_.update(deltaSeconds, input);
    }

    void Character::draw(render::ShaderProgram &shader) const
    {
        shader.setMat4("uModel", transformMatrix());
        mesh_.draw();
    }

    void Character::drawShadow(render::ShaderProgram &shader, const glm::mat4 &shadowMatrix) const
    {
        const glm::mat4 floorLift = glm::translate(glm::mat4(1.0F), {0.0F, 0.015F, 0.0F});
        shader.setMat4("uModel", floorLift * shadowMatrix * transformMatrix());
        mesh_.draw();
    }

} // namespace matrixalchemy::scene
