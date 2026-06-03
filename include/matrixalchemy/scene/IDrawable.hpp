#pragma once

#include "matrixalchemy/render/ShaderProgram.hpp"

namespace matrixalchemy::scene
{

    class IDrawable
    {
    public:
        virtual ~IDrawable() = default;

        virtual void draw(render::ShaderProgram &shader) const = 0;
    };

} // namespace matrixalchemy::scene
