#pragma once

#include <glm/glm.hpp>

namespace matrixalchemy::scene
{

    class OrbitCamera
    {
    public:
        [[nodiscard]] glm::vec3 position() const;
        [[nodiscard]] glm::mat4 viewMatrix() const;

        void beginDrag(double x, double y);
        void drag(double x, double y);
        void endDrag();
        void zoom(double wheelDelta);

        [[nodiscard]] float radius() const { return radius_; }
        [[nodiscard]] float thetaDegrees() const { return thetaDegrees_; }
        [[nodiscard]] float phiDegrees() const { return phiDegrees_; }

    private:
        glm::vec3 target_ = {0.0F, 0.5F, 0.0F};
        float radius_ = 10.0F;
        float thetaDegrees_ = -35.0F;
        float phiDegrees_ = 25.0F;
        double previousMouseX_ = 0.0;
        double previousMouseY_ = 0.0;
        bool dragging_ = false;
    };

} // namespace matrixalchemy::scene
