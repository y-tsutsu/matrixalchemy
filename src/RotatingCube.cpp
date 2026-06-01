#include "matrixalchemy/RotatingCube.hpp"

#include "matrixalchemy/Gl.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <array>

namespace
{

    float radians(float degrees)
    {
        return degrees * 3.14159265358979323846F / 180.0F;
    }

} // namespace

namespace matrixalchemy
{

    void RotatingCube::create(float size)
    {
        const float half = size / 2.0F;
        const glm::vec3 red = {0.90F, 0.20F, 0.25F};
        const glm::vec3 blue = {0.20F, 0.60F, 0.95F};
        const glm::vec3 green = {0.25F, 0.80F, 0.40F};
        const glm::vec3 yellow = {0.95F, 0.70F, 0.20F};
        const glm::vec3 purple = {0.65F, 0.35F, 0.85F};
        const glm::vec3 cyan = {0.15F, 0.75F, 0.80F};

        const std::array<ColoredVertex, 36> vertices = {
            ColoredVertex{{-half, -half, -half}, red},
            ColoredVertex{{half, -half, -half}, red},
            ColoredVertex{{half, half, -half}, red},
            ColoredVertex{{half, half, -half}, red},
            ColoredVertex{{-half, half, -half}, red},
            ColoredVertex{{-half, -half, -half}, red},

            ColoredVertex{{-half, -half, half}, blue},
            ColoredVertex{{half, -half, half}, blue},
            ColoredVertex{{half, half, half}, blue},
            ColoredVertex{{half, half, half}, blue},
            ColoredVertex{{-half, half, half}, blue},
            ColoredVertex{{-half, -half, half}, blue},

            ColoredVertex{{-half, half, half}, green},
            ColoredVertex{{-half, half, -half}, green},
            ColoredVertex{{-half, -half, -half}, green},
            ColoredVertex{{-half, -half, -half}, green},
            ColoredVertex{{-half, -half, half}, green},
            ColoredVertex{{-half, half, half}, green},

            ColoredVertex{{half, half, half}, yellow},
            ColoredVertex{{half, half, -half}, yellow},
            ColoredVertex{{half, -half, -half}, yellow},
            ColoredVertex{{half, -half, -half}, yellow},
            ColoredVertex{{half, -half, half}, yellow},
            ColoredVertex{{half, half, half}, yellow},

            ColoredVertex{{-half, -half, -half}, purple},
            ColoredVertex{{half, -half, -half}, purple},
            ColoredVertex{{half, -half, half}, purple},
            ColoredVertex{{half, -half, half}, purple},
            ColoredVertex{{-half, -half, half}, purple},
            ColoredVertex{{-half, -half, -half}, purple},

            ColoredVertex{{-half, half, -half}, cyan},
            ColoredVertex{{half, half, -half}, cyan},
            ColoredVertex{{half, half, half}, cyan},
            ColoredVertex{{half, half, half}, cyan},
            ColoredVertex{{-half, half, half}, cyan},
            ColoredVertex{{-half, half, -half}, cyan},
        };

        mesh_.upload(vertices, GL_TRIANGLES);
    }

    void RotatingCube::release()
    {
        mesh_.release();
    }

    void RotatingCube::update(float deltaSeconds)
    {
        rotationDegrees_ += 45.0F * deltaSeconds;
        if (rotationDegrees_ >= 360.0F)
        {
            rotationDegrees_ -= 360.0F;
        }
    }

    void RotatingCube::draw(ShaderProgram &shader) const
    {
        const glm::mat4 model = glm::translate(glm::mat4(1.0F), {0.0F, 0.5F, 0.0F}) * glm::rotate(glm::mat4(1.0F), radians(rotationDegrees_), {0.0F, 1.0F, 0.0F});

        shader.setMat4("uModel", model);
        mesh_.draw();
    }

} // namespace matrixalchemy
