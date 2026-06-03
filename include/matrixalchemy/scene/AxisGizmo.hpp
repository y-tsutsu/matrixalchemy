#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/scene/Drawable.hpp"

namespace matrixalchemy::scene
{

    class AxisGizmo final : public Drawable
    {
    public:
        void create(float length);
        void release();
        void draw(render::ShaderProgram &shader) const override;

    private:
        render::ColoredMesh mesh_;
    };

} // namespace matrixalchemy::scene
