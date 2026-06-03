#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/render/ShaderProgram.hpp"

namespace matrixalchemy
{

    class GridFloor
    {
    public:
        void create(float halfSize, int tileCount);
        void release();
        void draw(ShaderProgram &shader) const;

    private:
        ColoredMesh mesh_;
    };

} // namespace matrixalchemy
