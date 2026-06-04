#pragma once

#include "matrixalchemy/render/ShaderProgram.hpp"
#include "matrixalchemy/scene/AxisGizmo.hpp"
#include "matrixalchemy/scene/Character.hpp"
#include "matrixalchemy/scene/FloatingCubes.hpp"
#include "matrixalchemy/scene/GridFloor.hpp"
#include "matrixalchemy/scene/LightMarker.hpp"
#include "matrixalchemy/scene/OrbitCamera.hpp"
#include "matrixalchemy/scene/VrmCharacter.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace matrixalchemy::app
{

    class App
    {
    public:
        App();
        ~App();

        App(const App &) = delete;
        App &operator=(const App &) = delete;

        int run();

        [[nodiscard]] float floatingCubesRotationDegrees() const { return floatingCubes_.rotationDegrees(); }
        [[nodiscard]] float cameraRadius() const { return camera_.radius(); }
        [[nodiscard]] float cameraThetaDegrees() const { return camera_.thetaDegrees(); }
        [[nodiscard]] float cameraPhiDegrees() const { return camera_.phiDegrees(); }
        [[nodiscard]] glm::vec3 characterPosition() const;
        [[nodiscard]] float characterRotationDegrees() const;
        [[nodiscard]] bool debugUiVisible() const { return showDebugUi_; }

    private:
        static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
        static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void cursorPositionCallback(GLFWwindow *window, double x, double y);
        static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
        static void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);

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

        render::ShaderProgram shader_;
        scene::GridFloor gridFloor_;
        scene::AxisGizmo axisGizmo_;
        scene::LightMarker lightMarker_;
        scene::FloatingCubes floatingCubes_;
        scene::Character fallbackCharacter_;
        scene::VrmCharacter vrmCharacter_;
        bool useVrmCharacter_ = false;
        scene::OrbitCamera camera_;
        scene::CharacterInput characterInput_;
    };

} // namespace matrixalchemy::app
