#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/scene/IDrawable.hpp"

namespace matrixalchemy::scene
{

    class ArcaneRing final : public IDrawable
    {
    public:
        void create(float radius, int segments);
        void release();
        void update(float deltaSeconds);
        void draw(render::ShaderProgram &shader) const override;

    private:
        render::ColoredMesh mesh_;
        float rotationDegrees_ = 0.0F;
    };

} // namespace matrixalchemy::scene
