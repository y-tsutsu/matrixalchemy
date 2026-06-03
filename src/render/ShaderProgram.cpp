#include "matrixalchemy/render/ShaderProgram.hpp"

#include "matrixalchemy/platform/Gl.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace matrixalchemy::render
{

    ShaderProgram::~ShaderProgram()
    {
        release();
    }

    ShaderProgram::ShaderProgram(ShaderProgram &&other) noexcept
        : id_(other.id_)
    {
        other.id_ = 0;
    }

    ShaderProgram &ShaderProgram::operator=(ShaderProgram &&other) noexcept
    {
        if (this != &other)
        {
            release();
            id_ = other.id_;
            other.id_ = 0;
        }
        return *this;
    }

    void ShaderProgram::create(std::string_view vertexSource, std::string_view fragmentSource)
    {
        const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
        const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

        const unsigned int program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        int success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (success == 0)
        {
            int logLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(static_cast<std::size_t>(logLength));
            glGetProgramInfoLog(program, logLength, nullptr, log.data());
            glDeleteProgram(program);
            throw std::runtime_error("Failed to link shader program: " + std::string(log.begin(), log.end()));
        }

        release();
        id_ = program;
    }

    void ShaderProgram::release()
    {
        if (id_ != 0)
        {
            glDeleteProgram(id_);
            id_ = 0;
        }
    }

    void ShaderProgram::use() const
    {
        glUseProgram(id_);
    }

    void ShaderProgram::setBool(std::string_view name, bool value) const
    {
        const std::string uniformName(name);
        const int location = glGetUniformLocation(id_, uniformName.c_str());
        glUniform1i(location, value ? 1 : 0);
    }

    void ShaderProgram::setFloat(std::string_view name, float value) const
    {
        const std::string uniformName(name);
        const int location = glGetUniformLocation(id_, uniformName.c_str());
        glUniform1f(location, value);
    }

    void ShaderProgram::setInt(std::string_view name, int value) const
    {
        const std::string uniformName(name);
        const int location = glGetUniformLocation(id_, uniformName.c_str());
        glUniform1i(location, value);
    }

    void ShaderProgram::setMat4(std::string_view name, const glm::mat4 &value) const
    {
        const std::string uniformName(name);
        const int location = glGetUniformLocation(id_, uniformName.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }

    void ShaderProgram::setVec4(std::string_view name, const glm::vec4 &value) const
    {
        const std::string uniformName(name);
        const int location = glGetUniformLocation(id_, uniformName.c_str());
        glUniform4fv(location, 1, glm::value_ptr(value));
    }

    unsigned int ShaderProgram::compileShader(unsigned int type, std::string_view source)
    {
        const unsigned int shader = glCreateShader(type);
        const char *sourceData = source.data();
        const int sourceLength = static_cast<int>(source.size());
        glShaderSource(shader, 1, &sourceData, &sourceLength);
        glCompileShader(shader);

        int success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success == 0)
        {
            int logLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(static_cast<std::size_t>(logLength));
            glGetShaderInfoLog(shader, logLength, nullptr, log.data());
            glDeleteShader(shader);
            throw std::runtime_error("Failed to compile shader: " + std::string(log.begin(), log.end()));
        }

        return shader;
    }

} // namespace matrixalchemy::render
