#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/scene/IDrawable.hpp"

#include <glm/glm.hpp>

namespace matrixalchemy::scene
{

    class ArcaneRing final : public IDrawable
    {
    public:
        void create(float radius, int segments);
        void release();
        void update(float deltaSeconds);
        void setCenter(const glm::vec3 &center);
        void draw(render::ShaderProgram &shader) const override;

    private:
        render::ColoredMesh mesh_;
        glm::vec3 center_ = {0.0F, 0.0F, 0.0F};
        float rotationDegrees_ = 0.0F;
    };

} // namespace matrixalchemy::scene
