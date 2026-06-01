#pragma once

#include "matrixalchemy/AxisGizmo.hpp"
#include "matrixalchemy/GridFloor.hpp"
#include "matrixalchemy/ShaderProgram.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <array>

namespace matrixalchemy
{

    class App
    {
    public:
        App();
        ~App();

        App(const App &) = delete;
        App &operator=(const App &) = delete;

        int run();

        [[nodiscard]] float cubeRotationDegrees() const { return cubeRotation_; }
        [[nodiscard]] float cameraRadius() const { return cameraRadius_; }
        [[nodiscard]] float cameraThetaDegrees() const { return cameraTheta_; }
        [[nodiscard]] float cameraPhiDegrees() const { return cameraPhi_; }
        [[nodiscard]] bool debugUiVisible() const { return showDebugUi_; }

    private:
        static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
        static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

        void initializeWindow();
        void initializeOpenGL();
        void initializeScene();
        void processInput();
        void update(float deltaSeconds);
        void render();
        void resize(int width, int height);
        void requestClose();

        GLFWwindow *window_ = nullptr;
        int width_ = 1280;
        int height_ = 720;
        bool showDebugUi_ = true;

        unsigned int cubeVao_ = 0;
        unsigned int cubeVbo_ = 0;
        ShaderProgram shader_;
        GridFloor gridFloor_;
        AxisGizmo axisGizmo_;

        glm::vec3 cameraTarget_ = {0.0F, 0.5F, 0.0F};
        float cameraRadius_ = 8.0F;
        float cameraTheta_ = 45.0F;
        float cameraPhi_ = 25.0F;
        float cubeRotation_ = 0.0F;
    };

} // namespace matrixalchemy
