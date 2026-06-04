#pragma once

#include <glm/glm.hpp>

#include <span>
#include <string_view>

namespace matrixalchemy::render
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
        void setBool(std::string_view name, bool value) const;
        void setFloat(std::string_view name, float value) const;
        void setInt(std::string_view name, int value) const;
        void setMat4(std::string_view name, const glm::mat4 &value) const;
        void setMat4Array(std::string_view name, std::span<const glm::mat4> values) const;
        void setVec3(std::string_view name, const glm::vec3 &value) const;
        void setVec4(std::string_view name, const glm::vec4 &value) const;

    private:
        static unsigned int compileShader(unsigned int type, std::string_view source);

        unsigned int id_ = 0;
    };

} // namespace matrixalchemy::render
