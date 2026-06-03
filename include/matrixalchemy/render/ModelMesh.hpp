#pragma once

#include <glm/glm.hpp>

#include <span>

namespace matrixalchemy::render
{

    struct ModelVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoord;
        glm::vec3 normal;
    };

    class ModelMesh
    {
    public:
        ModelMesh() = default;
        ~ModelMesh();

        ModelMesh(const ModelMesh &) = delete;
        ModelMesh &operator=(const ModelMesh &) = delete;

        ModelMesh(ModelMesh &&other) noexcept;
        ModelMesh &operator=(ModelMesh &&other) noexcept;

        void upload(std::span<const ModelVertex> vertices, unsigned int primitiveType);
        void release();
        void draw() const;

    private:
        unsigned int vao_ = 0;
        unsigned int vbo_ = 0;
        unsigned int primitiveType_ = 0;
        int vertexCount_ = 0;
    };

} // namespace matrixalchemy::render
