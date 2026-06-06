#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/scene/SceneObject.hpp"

#include <glm/glm.hpp>

namespace matrixalchemy::scene
{

    class ArcaneRing final : public SceneObject
    {
    public:
        void create(float radius, int segments);
        void release() override;
        void update(float deltaSeconds) override;
        void setCenter(const glm::vec3 &center);
        void draw(render::ShaderProgram &shader) const override;

    private:
        render::ColoredMesh mesh_;
        glm::vec3 center_ = {0.0F, 0.0F, 0.0F};
        float rotationDegrees_ = 0.0F;
    };

} // namespace matrixalchemy::scene
