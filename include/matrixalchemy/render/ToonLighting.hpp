#pragma once

#include <glm/glm.hpp>

namespace matrixalchemy::render
{

    struct ToonLighting
    {
        bool enabled = true;
        bool useMaterialShadeColor = true;
        glm::vec3 shadeColor = {0.78F, 0.68F, 0.60F};
        float threshold = 0.62F;
        float softness = 0.10F;
        float litStrength = 1.08F;
    };

} // namespace matrixalchemy::render
