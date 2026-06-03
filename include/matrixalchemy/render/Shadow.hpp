#pragma once

#include <glm/glm.hpp>

namespace matrixalchemy
{

    glm::mat4 planarShadowMatrix(const glm::vec4 &plane, const glm::vec4 &light);

} // namespace matrixalchemy
