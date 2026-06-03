#include "matrixalchemy/render/ModelMesh.hpp"

#include "matrixalchemy/platform/Gl.hpp"

#include <cstddef>
#include <utility>

namespace matrixalchemy::render
{

    ModelMesh::~ModelMesh()
    {
        release();
    }

    ModelMesh::ModelMesh(ModelMesh &&other) noexcept
        : vao_(std::exchange(other.vao_, 0)), vbo_(std::exchange(other.vbo_, 0)), primitiveType_(std::exchange(other.primitiveType_, 0)), vertexCount_(std::exchange(other.vertexCount_, 0))
    {
    }

    ModelMesh &ModelMesh::operator=(ModelMesh &&other) noexcept
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

    void ModelMesh::upload(std::span<const ModelVertex> vertices, unsigned int primitiveType)
    {
        release();

        primitiveType_ = primitiveType;
        vertexCount_ = static_cast<int>(vertices.size());

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, static_cast<long>(vertices.size_bytes()), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), reinterpret_cast<void *>(offsetof(ModelVertex, position)));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), reinterpret_cast<void *>(offsetof(ModelVertex, color)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), reinterpret_cast<void *>(offsetof(ModelVertex, texCoord)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), reinterpret_cast<void *>(offsetof(ModelVertex, normal)));
        glEnableVertexAttribArray(3);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void ModelMesh::release()
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

    void ModelMesh::draw() const
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
