#pragma once

#include "matrixalchemy/ColoredMesh.hpp"
#include "matrixalchemy/ShaderProgram.hpp"

namespace matrixalchemy
{

    class AxisGizmo
    {
    public:
        void create(float length);
        void release();
        void draw(ShaderProgram &shader) const;

    private:
        ColoredMesh mesh_;
    };

} // namespace matrixalchemy
