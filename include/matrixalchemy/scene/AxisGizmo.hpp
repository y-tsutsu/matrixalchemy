#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/render/ShaderProgram.hpp"

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
