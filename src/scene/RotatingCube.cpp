#include "matrixalchemy/scene/RotatingCube.hpp"

#include "matrixalchemy/platform/Gl.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

namespace
{

    float radians(float degrees)
    {
        return degrees * 3.14159265358979323846F / 180.0F;
    }

    float wrapDegrees(float degrees)
    {
        const float wrapped = std::fmod(degrees, 360.0F);
        return wrapped < 0.0F ? wrapped + 360.0F : wrapped;
    }

    float randomFloat(std::mt19937 &random, float min, float max)
    {
        return std::uniform_real_distribution<float>(min, max)(random);
    }

    glm::vec3 randomColor(std::mt19937 &random)
    {
        constexpr std::array<glm::vec3, 12> palette = {
            glm::vec3{0.90F, 0.12F, 0.22F},
            glm::vec3{0.95F, 0.78F, 0.24F},
            glm::vec3{0.20F, 0.72F, 0.85F},
            glm::vec3{0.12F, 0.84F, 0.34F},
            glm::vec3{0.70F, 0.16F, 0.82F},
            glm::vec3{0.22F, 0.40F, 0.92F},
            glm::vec3{0.58F, 0.38F, 0.14F},
            glm::vec3{0.52F, 0.54F, 0.44F},
            glm::vec3{0.32F, 0.90F, 0.78F},
            glm::vec3{0.88F, 0.24F, 0.52F},
            glm::vec3{0.10F, 0.24F, 0.55F},
            glm::vec3{0.50F, 0.78F, 1.00F},
        };
        return palette[std::uniform_int_distribution<std::size_t>(0, palette.size() - 1)(random)];
    }

} // namespace

namespace matrixalchemy::scene
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

        const std::array<render::ColoredVertex, 36> vertices = {
            render::ColoredVertex{{-half, -half, -half}, red},
            render::ColoredVertex{{half, -half, -half}, red},
            render::ColoredVertex{{half, half, -half}, red},
            render::ColoredVertex{{half, half, -half}, red},
            render::ColoredVertex{{-half, half, -half}, red},
            render::ColoredVertex{{-half, -half, -half}, red},

            render::ColoredVertex{{-half, -half, half}, blue},
            render::ColoredVertex{{half, -half, half}, blue},
            render::ColoredVertex{{half, half, half}, blue},
            render::ColoredVertex{{half, half, half}, blue},
            render::ColoredVertex{{-half, half, half}, blue},
            render::ColoredVertex{{-half, -half, half}, blue},

            render::ColoredVertex{{-half, half, half}, green},
            render::ColoredVertex{{-half, half, -half}, green},
            render::ColoredVertex{{-half, -half, -half}, green},
            render::ColoredVertex{{-half, -half, -half}, green},
            render::ColoredVertex{{-half, -half, half}, green},
            render::ColoredVertex{{-half, half, half}, green},

            render::ColoredVertex{{half, half, half}, yellow},
            render::ColoredVertex{{half, half, -half}, yellow},
            render::ColoredVertex{{half, -half, -half}, yellow},
            render::ColoredVertex{{half, -half, -half}, yellow},
            render::ColoredVertex{{half, -half, half}, yellow},
            render::ColoredVertex{{half, half, half}, yellow},

            render::ColoredVertex{{-half, -half, -half}, purple},
            render::ColoredVertex{{half, -half, -half}, purple},
            render::ColoredVertex{{half, -half, half}, purple},
            render::ColoredVertex{{half, -half, half}, purple},
            render::ColoredVertex{{-half, -half, half}, purple},
            render::ColoredVertex{{-half, -half, -half}, purple},

            render::ColoredVertex{{-half, half, -half}, cyan},
            render::ColoredVertex{{half, half, -half}, cyan},
            render::ColoredVertex{{half, half, half}, cyan},
            render::ColoredVertex{{half, half, half}, cyan},
            render::ColoredVertex{{-half, half, half}, cyan},
            render::ColoredVertex{{-half, half, -half}, cyan},
        };

        mesh_.upload(vertices, GL_TRIANGLES);

        std::random_device seed;
        std::mt19937 random(seed());
        cubes_.clear();
        cubes_.reserve(20);

        for (int index = 0; index < 20; ++index)
        {
            const float cubeSize = randomFloat(random, 0.34F, 0.52F);
            const float minimumHeight = std::sqrt(3.0F) * cubeSize * 0.5F + 0.06F;
            const float x = randomFloat(random, -4.2F, 4.2F);
            const float z = randomFloat(random, -4.2F, 4.2F);

            CubeInstance cube;
            cube.basePosition = {x, randomFloat(random, minimumHeight + 0.25F, 3.45F), z};
            cube.colorScale = randomColor(random);
            cube.rotationAxis = glm::normalize(glm::vec3{
                randomFloat(random, 0.15F, 1.0F),
                randomFloat(random, 0.25F, 1.0F),
                randomFloat(random, 0.15F, 1.0F),
            });
            cube.size = cubeSize;
            cube.orbitRadius = randomFloat(random, 0.45F, 0.90F);
            cube.orbitSpeed = randomFloat(random, 0.70F, 1.50F) * (index % 2 == 0 ? 1.0F : -1.0F);
            cube.bobHeight = randomFloat(random, 0.25F, 0.55F);
            cube.bobSpeed = randomFloat(random, 1.60F, 2.85F);
            cube.phase = randomFloat(random, 0.0F, 6.28318530718F);
            cubes_.push_back(cube);
        }
    }

    void RotatingCube::release()
    {
        cubes_.clear();
        mesh_.release();
    }

    void RotatingCube::update(float deltaSeconds)
    {
        elapsedSeconds_ += deltaSeconds;
        rotationDegrees_ += 45.0F * deltaSeconds;
    }

    float RotatingCube::rotationDegrees() const
    {
        return wrapDegrees(rotationDegrees_);
    }

    void RotatingCube::draw(render::ShaderProgram &shader) const
    {
        for (const CubeInstance &cube : cubes_)
        {
            shader.setVec4("uColorOverride", {cube.colorScale, 1.0F});
            shader.setBool("uUseColorOverride", true);
            shader.setMat4("uModel", modelMatrix(cube));
            mesh_.draw();
        }
        shader.setBool("uUseColorOverride", false);
    }

    void RotatingCube::drawShadow(render::ShaderProgram &shader, const glm::mat4 &shadowMatrix) const
    {
        const glm::mat4 floorLift = glm::translate(glm::mat4(1.0F), {0.0F, 0.015F, 0.0F});
        for (const CubeInstance &cube : cubes_)
        {
            shader.setMat4("uModel", floorLift * shadowMatrix * modelMatrix(cube));
            mesh_.draw();
        }
    }

    glm::mat4 RotatingCube::modelMatrix(const CubeInstance &cube) const
    {
        const float orbitAngle = elapsedSeconds_ * cube.orbitSpeed + cube.phase;
        glm::vec3 position = cube.basePosition + glm::vec3{
                                                     std::cos(orbitAngle) * cube.orbitRadius,
                                                     std::sin(elapsedSeconds_ * cube.bobSpeed + cube.phase) * cube.bobHeight,
                                                     std::sin(orbitAngle) * cube.orbitRadius,
                                                 };
        position.y = std::max(position.y, std::sqrt(3.0F) * cube.size * 0.5F + 0.06F);
        const float rotation = radians(rotationDegrees_ * (1.0F + cube.orbitRadius) + cube.phase * 60.0F);

        return glm::translate(glm::mat4(1.0F), position) *
               glm::rotate(glm::mat4(1.0F), rotation, glm::normalize(cube.rotationAxis)) *
               glm::scale(glm::mat4(1.0F), glm::vec3(cube.size));
    }

} // namespace matrixalchemy::scene
