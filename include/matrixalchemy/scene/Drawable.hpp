#pragma once

#include "matrixalchemy/render/ShaderProgram.hpp"

namespace matrixalchemy::scene
{

    class Drawable
    {
    public:
        virtual ~Drawable() = default;

        virtual void draw(render::ShaderProgram &shader) const = 0;
    };

} // namespace matrixalchemy::scene
