#include "matrixalchemy/render/ColoredMesh.hpp"

#include "matrixalchemy/platform/Gl.hpp"

#include <utility>

namespace matrixalchemy::render
{

    ColoredMesh::~ColoredMesh()
    {
        release();
    }

    ColoredMesh::ColoredMesh(ColoredMesh &&other) noexcept
        : vao_(std::exchange(other.vao_, 0)), vbo_(std::exchange(other.vbo_, 0)), primitiveType_(std::exchange(other.primitiveType_, 0)), vertexCount_(std::exchange(other.vertexCount_, 0))
    {
    }

    ColoredMesh &ColoredMesh::operator=(ColoredMesh &&other) noexcept
    {
        if (this != &other)
        {
            release();
            vao_ = std::exchange(other.vao_, 0);
            vbo_ = std::exchange(other.vbo_, 0);
            primitiveType_ = std::exchange(other.primitiveType_, 0);
            vertexCount_ = std::exchange(other.vertexCount_, 0);
        }
        return *this;
    }

    void ColoredMesh::upload(std::span<const ColoredVertex> vertices, unsigned int primitiveType)
    {
        release();

        primitiveType_ = primitiveType;
        vertexCount_ = static_cast<int>(vertices.size());

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, static_cast<long>(vertices.size_bytes()), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), reinterpret_cast<void *>(sizeof(glm::vec3)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void ColoredMesh::release()
    {
        if (vbo_ != 0)
        {
            glDeleteBuffers(1, &vbo_);
            vbo_ = 0;
        }
        if (vao_ != 0)
        {
            glDeleteVertexArrays(1, &vao_);
            vao_ = 0;
        }
        primitiveType_ = 0;
        vertexCount_ = 0;
    }

    void ColoredMesh::draw() const
    {
        if (vertexCount_ == 0)
        {
            return;
        }

        glBindVertexArray(vao_);
        glDrawArrays(primitiveType_, 0, vertexCount_);
        glBindVertexArray(0);
    }

} // namespace matrixalchemy::render
