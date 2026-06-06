#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/scene/SceneObject.hpp"

namespace matrixalchemy::scene
{

    class AxisGizmo final : public SceneObject
    {
    public:
        void create(float length);
        void release() override;
        void draw(render::ShaderProgram &shader) const override;

    private:
        render::ColoredMesh mesh_;
    };

} // namespace matrixalchemy::scene
