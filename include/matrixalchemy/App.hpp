#pragma once

#include "matrixalchemy/AxisGizmo.hpp"
#include "matrixalchemy/Character.hpp"
#include "matrixalchemy/GridFloor.hpp"
#include "matrixalchemy/Model.hpp"
#include "matrixalchemy/OrbitCamera.hpp"
#include "matrixalchemy/RotatingCube.hpp"
#include "matrixalchemy/ShaderProgram.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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

        [[nodiscard]] float cubeRotationDegrees() const { return cube_.rotationDegrees(); }
        [[nodiscard]] float cameraRadius() const { return camera_.radius(); }
        [[nodiscard]] float cameraThetaDegrees() const { return camera_.thetaDegrees(); }
        [[nodiscard]] float cameraPhiDegrees() const { return camera_.phiDegrees(); }
        [[nodiscard]] glm::vec3 characterPosition() const { return character_.position(); }
        [[nodiscard]] float characterRotationDegrees() const { return character_.rotationDegrees(); }
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

        ShaderProgram shader_;
        GridFloor gridFloor_;
        AxisGizmo axisGizmo_;
        RotatingCube cube_;
        Character character_;
        Model sampleModel_;
        bool useCharacterModel_ = false;
        OrbitCamera camera_;
        CharacterInput characterInput_;
    };

} // namespace matrixalchemy
