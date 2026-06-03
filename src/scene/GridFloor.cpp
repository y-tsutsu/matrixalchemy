#include "matrixalchemy/scene/GridFloor.hpp"

#include "matrixalchemy/platform/Gl.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace matrixalchemy::scene
{

    void GridFloor::create(float halfSize, int tileCount)
    {
        const float tileSize = halfSize * 2.0F / static_cast<float>(tileCount);
        const glm::vec3 colorA = {0.05F, 0.48F, 0.10F};
        const glm::vec3 colorB = {0.82F, 0.86F, 0.82F};

        std::vector<render::ColoredVertex> vertices;
        vertices.reserve(static_cast<std::size_t>(tileCount * tileCount * 6));

        for (int z = 0; z < tileCount; ++z)
        {
            for (int x = 0; x < tileCount; ++x)
            {
                const float left = -halfSize + static_cast<float>(x) * tileSize;
                const float right = left + tileSize;
                const float near = -halfSize + static_cast<float>(z) * tileSize;
                const float far = near + tileSize;
                const glm::vec3 color = ((x + z) % 2 == 0) ? colorA : colorB;

                vertices.push_back({{left, 0.0F, near}, color});
                vertices.push_back({{right, 0.0F, near}, color});
                vertices.push_back({{left, 0.0F, far}, color});
                vertices.push_back({{right, 0.0F, near}, color});
                vertices.push_back({{right, 0.0F, far}, color});
                vertices.push_back({{left, 0.0F, far}, color});
            }
        }

        mesh_.upload(vertices, GL_TRIANGLES);
    }

    void GridFloor::release()
    {
        mesh_.release();
    }

    void GridFloor::draw(render::ShaderProgram &shader) const
    {
        shader.setMat4("uModel", glm::mat4(1.0F));
        mesh_.draw();
    }

} // namespace matrixalchemy::scene
