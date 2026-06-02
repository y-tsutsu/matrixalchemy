#pragma once

#include "matrixalchemy/ColoredMesh.hpp"
#include "matrixalchemy/ShaderProgram.hpp"

#include <filesystem>
#include <glm/glm.hpp>
#include <vector>

namespace matrixalchemy
{

    class Model
    {
    public:
        void load(const std::filesystem::path &path, const glm::vec3 &color);
        void release();
        void draw(ShaderProgram &shader, const glm::mat4 &modelMatrix) const;

        [[nodiscard]] bool empty() const { return meshes_.empty(); }

    private:
        std::vector<ColoredMesh> meshes_;
    };

} // namespace matrixalchemy
