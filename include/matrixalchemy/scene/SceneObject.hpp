#pragma once

#include "matrixalchemy/render/ShaderProgram.hpp"

#include <glm/glm.hpp>

namespace matrixalchemy::scene
{

    class SceneObject
    {
    public:
        virtual ~SceneObject() = default;

        virtual void update(float) {}
        virtual void draw(render::ShaderProgram &shader) const = 0;
        virtual void drawShadow(render::ShaderProgram &, const glm::mat4 &) const {}
        virtual void release() = 0;
    };

} // namespace matrixalchemy::scene
