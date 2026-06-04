#include "matrixalchemy/scene/LightMarker.hpp"

#include "matrixalchemy/platform/Gl.hpp"
#include "matrixalchemy/render/ShaderProgram.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <vector>

namespace
{

    constexpr float pi = 3.14159265358979323846F;

    float radians(float degrees)
    {
        return degrees * pi / 180.0F;
    }

} // namespace

namespace matrixalchemy::scene
{

    void LightMarker::create(float radius, int slices, int stacks)
    {
        const glm::vec3 color = {1.00F, 0.86F, 0.08F};
        std::vector<render::ColoredVertex> vertices;
        vertices.reserve(static_cast<std::size_t>(slices * stacks * 6));

        for (int stack = 0; stack < stacks; ++stack)
        {
            const float v0 = static_cast<float>(stack) / static_cast<float>(stacks);
            const float v1 = static_cast<float>(stack + 1) / static_cast<float>(stacks);
            const float phi0 = (v0 - 0.5F) * pi;
            const float phi1 = (v1 - 0.5F) * pi;

            for (int slice = 0; slice < slices; ++slice)
            {
                const float u0 = static_cast<float>(slice) / static_cast<float>(slices);
                const float u1 = static_cast<float>(slice + 1) / static_cast<float>(slices);
                const float theta0 = u0 * pi * 2.0F;
                const float theta1 = u1 * pi * 2.0F;

                const auto point = [radius](float theta, float phi)
                {
                    const float cosPhi = std::cos(phi);
                    return glm::vec3{
                        std::cos(theta) * cosPhi * radius,
                        std::sin(phi) * radius,
                        std::sin(theta) * cosPhi * radius,
                    };
                };

                const glm::vec3 p00 = point(theta0, phi0);
                const glm::vec3 p10 = point(theta1, phi0);
                const glm::vec3 p01 = point(theta0, phi1);
                const glm::vec3 p11 = point(theta1, phi1);

                vertices.push_back({p00, color});
                vertices.push_back({p10, color});
                vertices.push_back({p01, color});
                vertices.push_back({p10, color});
                vertices.push_back({p11, color});
                vertices.push_back({p01, color});
            }
        }

        mesh_.upload(vertices, GL_TRIANGLES);
    }

    void LightMarker::release()
    {
        mesh_.release();
    }

    void LightMarker::update(float deltaSeconds)
    {
        constexpr float orbitSpeed = 35.0F;
        constexpr float orbitRadius = 5.3F;
        constexpr float height = 5.6F;

        orbitDegrees_ += orbitSpeed * deltaSeconds;
        const float angle = radians(orbitDegrees_);
        position_ = {std::cos(angle) * orbitRadius, height, std::sin(angle) * orbitRadius};
    }

    void LightMarker::draw(render::ShaderProgram &shader) const
    {
        shader.setMat4("uModel", glm::translate(glm::mat4(1.0F), position_));
        mesh_.draw();
    }

} // namespace matrixalchemy::scene
