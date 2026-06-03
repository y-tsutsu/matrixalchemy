#pragma once

#include <glm/glm.hpp>

#include <span>
#include <vector>

namespace matrixalchemy
{

    struct ColoredVertex
    {
        glm::vec3 position;
        glm::vec3 color;
    };

    class ColoredMesh
    {
    public:
        ColoredMesh() = default;
        ~ColoredMesh();

        ColoredMesh(const ColoredMesh &) = delete;
        ColoredMesh &operator=(const ColoredMesh &) = delete;

        ColoredMesh(ColoredMesh &&other) noexcept;
        ColoredMesh &operator=(ColoredMesh &&other) noexcept;

        void upload(std::span<const ColoredVertex> vertices, unsigned int primitiveType);
        void release();
        void draw() const;

    private:
        unsigned int vao_ = 0;
        unsigned int vbo_ = 0;
        unsigned int primitiveType_ = 0;
        int vertexCount_ = 0;
    };

} // namespace matrixalchemy
