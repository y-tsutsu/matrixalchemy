#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/scene/SceneObject.hpp"

namespace matrixalchemy::scene
{

    class GridFloor final : public SceneObject
    {
    public:
        void create(float halfSize, int tileCount);
        void release() override;
        void draw(render::ShaderProgram &shader) const override;

    private:
        render::ColoredMesh mesh_;
    };

} // namespace matrixalchemy::scene
