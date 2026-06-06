#pragma once

#include "matrixalchemy/render/ColoredMesh.hpp"
#include "matrixalchemy/scene/SceneObject.hpp"

#include <glm/glm.hpp>

namespace matrixalchemy::scene
{

    class LightMarker final : public SceneObject
    {
    public:
        void create(float radius, int slices, int stacks);
        void release() override;
        void update(float deltaSeconds) override;
        void setPosition(const glm::vec3 &position) { position_ = position; }
        [[nodiscard]] glm::vec3 position() const { return position_; }
        void draw(render::ShaderProgram &shader) const override;

    private:
        render::ColoredMesh mesh_;
        glm::vec3 position_ = {0.0F, 0.0F, 0.0F};
        float orbitDegrees_ = 215.0F;
    };

} // namespace matrixalchemy::scene
