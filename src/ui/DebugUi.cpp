#include "matrixalchemy/ui/DebugUi.hpp"

#include "matrixalchemy/app/App.hpp"
#include "matrixalchemy/platform/Gl.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cmath>

namespace
{

    float wrapDegrees(float degrees)
    {
        const float wrapped = std::fmod(degrees, 360.0F);
        return wrapped < 0.0F ? wrapped + 360.0F : wrapped;
    }

} // namespace

namespace matrixalchemy::ui
{

    void DebugUi::initialize(GLFWwindow *window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void DebugUi::beginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void DebugUi::draw(app::App &app)
    {
        ImGui::Begin("Matrix Alchemy");
        ImGui::Text("F1: toggle this panel");
        ImGui::Separator();
        ImGui::Text("Floating cubes rotation: %.2f deg", app.floatingCubesRotationDegrees());
        ImGui::Separator();
        ImGui::Text("Camera radius: %.2f", app.cameraRadius());
        ImGui::Text("Camera theta: %.2f deg", wrapDegrees(app.cameraThetaDegrees()));
        ImGui::Text("Camera phi: %.2f deg", app.cameraPhiDegrees());
        ImGui::Separator();
        const glm::vec3 position = app.characterPosition();
        ImGui::Text("Character position: %.2f, %.2f, %.2f", position.x, position.y, position.z);
        ImGui::Text("Character render height: %.2f", app.characterRenderHeight());
        ImGui::Text("Character rotation: %.2f deg", wrapDegrees(app.characterRotationDegrees()));
        ImGui::Separator();
        auto &poseSettings = app.poseAnimationSettings();
        ImGui::Checkbox("Arm animation", &poseSettings.enabled);
        ImGui::SliderFloat("Arm speed", &poseSettings.speed, 0.1F, 4.0F, "%.2f");
        ImGui::SliderFloat("Arm base", &poseSettings.baseArmAngleDegrees, 0.0F, 90.0F, "%.1f deg");
        ImGui::SliderFloat("Arm spread", &poseSettings.spreadAngleDegrees, 0.0F, 70.0F, "%.1f deg");
        ImGui::Checkbox("Head animation", &poseSettings.headEnabled);
        ImGui::SliderFloat("Head yaw", &poseSettings.headYawDegrees, 0.0F, 25.0F, "%.1f deg");
        ImGui::Checkbox("Tail animation", &poseSettings.tailEnabled);
        ImGui::SliderFloat("Tail swing", &poseSettings.tailSwingDegrees, 0.0F, 35.0F, "%.1f deg");
        ImGui::Separator();
        auto &toonLighting = app.toonLighting();
        ImGui::Checkbox("Toon lighting", &toonLighting.enabled);
        ImGui::ColorEdit3("Shade color", glm::value_ptr(toonLighting.shadeColor));
        ImGui::SliderFloat("Shade threshold", &toonLighting.threshold, 0.0F, 1.0F, "%.2f");
        ImGui::SliderFloat("Shade softness", &toonLighting.softness, 0.0F, 0.35F, "%.2f");
        ImGui::SliderFloat("Lit strength", &toonLighting.litStrength, 0.5F, 1.5F, "%.2f");
        ImGui::End();
    }

    void DebugUi::render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void DebugUi::shutdown()
    {
        if (ImGui::GetCurrentContext() == nullptr)
        {
            return;
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

} // namespace matrixalchemy::ui
