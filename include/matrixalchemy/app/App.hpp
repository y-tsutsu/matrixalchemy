#pragma once

#include "matrixalchemy/render/ShaderProgram.hpp"
#include "matrixalchemy/render/ToonLighting.hpp"
#include "matrixalchemy/scene/ArcaneRing.hpp"
#include "matrixalchemy/scene/AxisGizmo.hpp"
#include "matrixalchemy/scene/Character.hpp"
#include "matrixalchemy/scene/FloatingCubes.hpp"
#include "matrixalchemy/scene/GridFloor.hpp"
#include "matrixalchemy/scene/LightMarker.hpp"
#include "matrixalchemy/scene/OrbitCamera.hpp"
#include "matrixalchemy/scene/SceneObject.hpp"
#include "matrixalchemy/scene/VrmCharacter.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>

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
        [[nodiscard]] float characterRenderHeight() const;
        [[nodiscard]] const asset::PoseAnimationSettings &poseAnimationSettings() const { return poseAnimationSettings_; }
        [[nodiscard]] asset::PoseAnimationSettings &poseAnimationSettings() { return poseAnimationSettings_; }
        [[nodiscard]] const render::ToonLighting &toonLighting() const { return toonLighting_; }
        [[nodiscard]] render::ToonLighting &toonLighting() { return toonLighting_; }
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
        void registerSceneObjects();
        void releaseSceneResources();
        void processInput();
        void update(float deltaSeconds);
        void render();
        void updateRegisteredSceneObjects(float deltaSeconds);
        void setupFrameShader(const glm::mat4 &projection, const glm::mat4 &view);
        void drawFloorMask();
        void drawProjectedShadows();
        void drawRegisteredSceneObjects();
        void drawActiveCharacter();
        void drawDebugUi();
        void resize(int width, int height);
        void requestClose();

        GLFWwindow *window_ = nullptr;
        int width_ = 1280;
        int height_ = 720;
        bool showDebugUi_ = true;

        render::ShaderProgram shader_;
        scene::GridFloor gridFloor_;
        scene::ArcaneRing arcaneRing_;
        scene::AxisGizmo axisGizmo_;
        scene::LightMarker lightMarker_;
        scene::FloatingCubes floatingCubes_;
        scene::Character fallbackCharacter_;
        scene::VrmCharacter vrmCharacter_;
        bool useVrmCharacter_ = false;
        scene::SceneObject *activeCharacter_ = nullptr;
        std::vector<scene::SceneObject *> sceneObjects_;
        scene::OrbitCamera camera_;
        scene::CharacterInput characterInput_;
        asset::PoseAnimationSettings poseAnimationSettings_;
        render::ToonLighting toonLighting_;
    };

} // namespace matrixalchemy::app
