#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/scene/IDrawable.hpp"

namespace matrixalchemy::scene
{

    class GridFloor final : public IDrawable
    {
    public:
        void create(float halfSize, int tileCount);
        void release();
        void draw(render::ShaderProgram &shader) const override;

    private:
        render::ColoredMesh mesh_;
    };

} // namespace matrixalchemy::scene
