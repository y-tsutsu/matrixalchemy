#include "matrixalchemy/scene/OrbitCamera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace
{

    float radians(float degrees)
    {
        return degrees * 3.14159265358979323846F / 180.0F;
    }

} // namespace

namespace matrixalchemy::scene
{

    glm::vec3 OrbitCamera::position() const
    {
        const float theta = radians(thetaDegrees_);
        const float phi = radians(phiDegrees_);
        return {
            radius_ * std::cos(theta) * std::cos(phi),
            radius_ * std::sin(phi),
            radius_ * std::sin(theta) * std::cos(phi),
        };
    }

    glm::mat4 OrbitCamera::viewMatrix() const
    {
        return glm::lookAt(position(), target_, {0.0F, 1.0F, 0.0F});
    }

    void OrbitCamera::beginDrag(double x, double y)
    {
        previousMouseX_ = x;
        previousMouseY_ = y;
        dragging_ = true;
    }

    void OrbitCamera::drag(double x, double y)
    {
        if (!dragging_)
        {
            return;
        }

        thetaDegrees_ -= static_cast<float>(x - previousMouseX_) * 0.4F;
        phiDegrees_ += static_cast<float>(y - previousMouseY_) * 0.4F;
        phiDegrees_ = std::clamp(phiDegrees_, -85.0F, 85.0F);

        previousMouseX_ = x;
        previousMouseY_ = y;
    }

    void OrbitCamera::endDrag()
    {
        dragging_ = false;
    }

    void OrbitCamera::zoom(double wheelDelta)
    {
        radius_ -= static_cast<float>(wheelDelta) * 0.6F;
        radius_ = std::clamp(radius_, 3.0F, 18.0F);
    }

} // namespace matrixalchemy::scene
