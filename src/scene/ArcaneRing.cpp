#include "matrixalchemy/scene/ArcaneRing.hpp"

#include "matrixalchemy/platform/Gl.hpp"
#include "matrixalchemy/render/ShaderProgram.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <vector>

namespace
{

    constexpr float pi = 3.14159265358979323846F;
    constexpr float ringHeight = 0.028F;
    constexpr float rotationSpeed = 24.0F;
    constexpr glm::vec3 cyan = {0.04F, 0.32F, 1.00F};
    constexpr glm::vec3 blue = {0.02F, 0.10F, 0.74F};
    constexpr glm::vec3 pale = {0.18F, 0.48F, 1.00F};

    float radians(float degrees)
    {
        return degrees * pi / 180.0F;
    }

    glm::vec3 ringPoint(float radius, float angle)
    {
        return {std::cos(angle) * radius, ringHeight, std::sin(angle) * radius};
    }

    void addCircle(std::vector<matrixalchemy::render::ColoredVertex> &vertices, float radius, int segments, const glm::vec3 &color)
    {
        for (int segment = 0; segment < segments; ++segment)
        {
            const float a0 = static_cast<float>(segment) / static_cast<float>(segments) * pi * 2.0F;
            const float a1 = static_cast<float>(segment + 1) / static_cast<float>(segments) * pi * 2.0F;
            vertices.push_back({ringPoint(radius, a0), color});
            vertices.push_back({ringPoint(radius, a1), color});
        }
    }

    void addRadialTicks(std::vector<matrixalchemy::render::ColoredVertex> &vertices, float innerRadius, float outerRadius, int count, const glm::vec3 &color)
    {
        for (int tick = 0; tick < count; ++tick)
        {
            const float angle = static_cast<float>(tick) / static_cast<float>(count) * pi * 2.0F;
            vertices.push_back({ringPoint(innerRadius, angle), color});
            vertices.push_back({ringPoint(outerRadius, angle), color});
        }
    }

    void addCrossMarks(std::vector<matrixalchemy::render::ColoredVertex> &vertices, float radius, int count, const glm::vec3 &color)
    {
        constexpr float markLength = 0.16F;
        for (int mark = 0; mark < count; ++mark)
        {
            const float angle = static_cast<float>(mark) / static_cast<float>(count) * pi * 2.0F;
            const glm::vec3 center = ringPoint(radius, angle);
            const glm::vec3 tangent = {-std::sin(angle), 0.0F, std::cos(angle)};
            vertices.push_back({center - tangent * markLength, color});
            vertices.push_back({center + tangent * markLength, color});
        }
    }

} // namespace

namespace matrixalchemy::scene
{

    void ArcaneRing::create(float radius, int segments)
    {
        std::vector<render::ColoredVertex> vertices;
        vertices.reserve(static_cast<std::size_t>(segments * 6 + 96));

        addCircle(vertices, radius, segments, cyan);
        addCircle(vertices, radius * 0.72F, segments, blue);
        addCircle(vertices, radius * 0.44F, segments, pale);
        addRadialTicks(vertices, radius * 0.78F, radius, 32, pale);
        addRadialTicks(vertices, radius * 0.45F, radius * 0.62F, 16, cyan);
        addCrossMarks(vertices, radius * 0.88F, 8, blue);

        mesh_.upload(vertices, GL_LINES);
    }

    void ArcaneRing::release()
    {
        mesh_.release();
    }

    void ArcaneRing::update(float deltaSeconds)
    {
        rotationDegrees_ += rotationSpeed * deltaSeconds;
    }

    void ArcaneRing::setCenter(const glm::vec3 &center)
    {
        center_ = {center.x, 0.0F, center.z};
    }

    void ArcaneRing::draw(render::ShaderProgram &shader) const
    {
        const bool previousBlend = glIsEnabled(GL_BLEND) == GL_TRUE;
        GLboolean previousDepthMask = GL_TRUE;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glLineWidth(2.0F);

        const glm::mat4 translation = glm::translate(glm::mat4(1.0F), center_);
        const glm::mat4 base = translation * glm::rotate(glm::mat4(1.0F), radians(rotationDegrees_), {0.0F, 1.0F, 0.0F});
        shader.setMat4("uModel", base);
        mesh_.draw();

        const glm::mat4 counter = translation * glm::rotate(glm::mat4(1.0F), radians(-rotationDegrees_ * 0.55F), {0.0F, 1.0F, 0.0F});
        shader.setMat4("uModel", counter);
        mesh_.draw();

        glLineWidth(1.0F);
        glDepthMask(previousDepthMask);
        if (previousBlend)
        {
            glEnable(GL_BLEND);
        }
        else
        {
            glDisable(GL_BLEND);
        }
    }

} // namespace matrixalchemy::scene
