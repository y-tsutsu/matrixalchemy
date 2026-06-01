#pragma once

#include <glm/glm.hpp>

#include <string_view>

namespace matrixalchemy
{

    class ShaderProgram
    {
    public:
        ShaderProgram() = default;
        ~ShaderProgram();

        ShaderProgram(const ShaderProgram &) = delete;
        ShaderProgram &operator=(const ShaderProgram &) = delete;

        ShaderProgram(ShaderProgram &&other) noexcept;
        ShaderProgram &operator=(ShaderProgram &&other) noexcept;

        void create(std::string_view vertexSource, std::string_view fragmentSource);
        void release();
        void use() const;
        void setMat4(std::string_view name, const glm::mat4 &value) const;

    private:
        static unsigned int compileShader(unsigned int type, std::string_view source);

        unsigned int id_ = 0;
    };

} // namespace matrixalchemy
