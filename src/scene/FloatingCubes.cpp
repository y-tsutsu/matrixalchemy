#include "matrixalchemy/scene/FloatingCubes.hpp"

#include "matrixalchemy/platform/Gl.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

namespace
{

    constexpr int cubeCount = 20;
    constexpr float minCubeSize = 0.34F;
    constexpr float maxCubeSize = 0.52F;
    constexpr float floorClearance = 0.06F;
    constexpr float floorRange = 4.2F;
    constexpr float minBaseLift = 0.25F;
    constexpr float maxBaseHeight = 3.45F;
    constexpr float minOrbitRadius = 0.45F;
    constexpr float maxOrbitRadius = 0.90F;
    constexpr float minOrbitSpeed = 0.70F;
    constexpr float maxOrbitSpeed = 1.50F;
    constexpr float minBobHeight = 0.25F;
    constexpr float maxBobHeight = 0.55F;
    constexpr float minBobSpeed = 1.60F;
    constexpr float maxBobSpeed = 2.85F;
    constexpr float twoPi = 6.28318530718F;

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

    float cubeFloorHeight(float size)
    {
        return std::sqrt(3.0F) * size * 0.5F + floorClearance;
    }

} // namespace

namespace matrixalchemy::scene
{

    void FloatingCubes::create(float size)
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
        cubes_.reserve(cubeCount);

        for (int index = 0; index < cubeCount; ++index)
        {
            const float cubeSize = randomFloat(random, minCubeSize, maxCubeSize);
            const float x = randomFloat(random, -floorRange, floorRange);
            const float z = randomFloat(random, -floorRange, floorRange);

            CubeInstance cube;
            cube.basePosition = {x, randomFloat(random, cubeFloorHeight(cubeSize) + minBaseLift, maxBaseHeight), z};
            cube.colorScale = randomColor(random);
            cube.rotationAxis = glm::normalize(glm::vec3{
                randomFloat(random, 0.15F, 1.0F),
                randomFloat(random, 0.25F, 1.0F),
                randomFloat(random, 0.15F, 1.0F),
            });
            cube.size = cubeSize;
            cube.orbitRadius = randomFloat(random, minOrbitRadius, maxOrbitRadius);
            cube.orbitSpeed = randomFloat(random, minOrbitSpeed, maxOrbitSpeed) * (index % 2 == 0 ? 1.0F : -1.0F);
            cube.bobHeight = randomFloat(random, minBobHeight, maxBobHeight);
            cube.bobSpeed = randomFloat(random, minBobSpeed, maxBobSpeed);
            cube.phase = randomFloat(random, 0.0F, twoPi);
            cubes_.push_back(cube);
        }
    }

    void FloatingCubes::release()
    {
        cubes_.clear();
        mesh_.release();
    }

    void FloatingCubes::update(float deltaSeconds)
    {
        elapsedSeconds_ += deltaSeconds;
        rotationDegrees_ += 45.0F * deltaSeconds;
    }

    float FloatingCubes::rotationDegrees() const
    {
        return wrapDegrees(rotationDegrees_);
    }

    void FloatingCubes::draw(render::ShaderProgram &shader) const
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

    void FloatingCubes::drawShadow(render::ShaderProgram &shader, const glm::mat4 &shadowMatrix) const
    {
        const glm::mat4 floorLift = glm::translate(glm::mat4(1.0F), {0.0F, 0.015F, 0.0F});
        for (const CubeInstance &cube : cubes_)
        {
            shader.setMat4("uModel", floorLift * shadowMatrix * modelMatrix(cube));
            mesh_.draw();
        }
    }

    glm::mat4 FloatingCubes::modelMatrix(const CubeInstance &cube) const
    {
        const float orbitAngle = elapsedSeconds_ * cube.orbitSpeed + cube.phase;
        glm::vec3 position = cube.basePosition + glm::vec3{
                                                     std::cos(orbitAngle) * cube.orbitRadius,
                                                     std::sin(elapsedSeconds_ * cube.bobSpeed + cube.phase) * cube.bobHeight,
                                                     std::sin(orbitAngle) * cube.orbitRadius,
                                                 };
        position.y = std::max(position.y, cubeFloorHeight(cube.size));
        const float rotation = radians(rotationDegrees_ * (1.0F + cube.orbitRadius) + cube.phase * 60.0F);

        return glm::translate(glm::mat4(1.0F), position) *
               glm::rotate(glm::mat4(1.0F), rotation, glm::normalize(cube.rotationAxis)) *
               glm::scale(glm::mat4(1.0F), glm::vec3(cube.size));
    }

} // namespace matrixalchemy::scene
