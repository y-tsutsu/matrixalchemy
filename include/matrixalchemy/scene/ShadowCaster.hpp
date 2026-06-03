#pragma once

#include "matrixalchemy/render/ShaderProgram.hpp"

#include <glm/glm.hpp>

namespace matrixalchemy::scene
{

    class ShadowCaster
    {
    public:
        virtual ~ShadowCaster() = default;

        virtual void drawShadow(render::ShaderProgram &shader, const glm::mat4 &shadowMatrix) const = 0;
    };

} // namespace matrixalchemy::scene
