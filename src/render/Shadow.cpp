#include "matrixalchemy/render/Shadow.hpp"

#include <stdexcept>

namespace matrixalchemy::render
{

    glm::mat4 planarShadowMatrix(const glm::vec4 &plane, const glm::vec4 &light)
    {
        const float dot = glm::dot(plane, light);
        if (dot == 0.0F)
        {
            throw std::runtime_error("Cannot create a planar shadow matrix from a light parallel to the plane.");
        }

        return glm::mat4(
            dot - light.x * plane.x,
            -light.y * plane.x,
            -light.z * plane.x,
            -light.w * plane.x,
            -light.x * plane.y,
            dot - light.y * plane.y,
            -light.z * plane.y,
            -light.w * plane.y,
            -light.x * plane.z,
            -light.y * plane.z,
            dot - light.z * plane.z,
            -light.w * plane.z,
            -light.x * plane.w,
            -light.y * plane.w,
            -light.z * plane.w,
            dot - light.w * plane.w);
    }

} // namespace matrixalchemy::render
