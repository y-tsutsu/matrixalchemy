#include "matrixalchemy/scene/AxisGizmo.hpp"

#include "matrixalchemy/platform/Gl.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <array>

namespace matrixalchemy::scene
{

    void AxisGizmo::create(float length)
    {
        constexpr glm::vec3 red = {0.95F, 0.10F, 0.10F};
        constexpr glm::vec3 green = {0.10F, 0.75F, 0.20F};
        constexpr glm::vec3 blue = {0.20F, 0.35F, 1.00F};

        const std::array<render::ColoredVertex, 6> vertices = {
            render::ColoredVertex{{0.0F, 0.02F, 0.0F}, red},
            render::ColoredVertex{{length, 0.02F, 0.0F}, red},
            render::ColoredVertex{{0.0F, 0.02F, 0.0F}, green},
            render::ColoredVertex{{0.0F, length, 0.0F}, green},
            render::ColoredVertex{{0.0F, 0.02F, 0.0F}, blue},
            render::ColoredVertex{{0.0F, 0.02F, length}, blue},
        };

        mesh_.upload(vertices, GL_LINES);
    }

    void AxisGizmo::release()
    {
        mesh_.release();
    }

    void AxisGizmo::draw(render::ShaderProgram &shader) const
    {
        shader.setMat4("uModel", glm::mat4(1.0F));
        glLineWidth(2.0F);
        mesh_.draw();
        glLineWidth(1.0F);
    }

} // namespace matrixalchemy::scene
