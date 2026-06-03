#include "matrixalchemy/ui/DebugUi.hpp"

#include "matrixalchemy/app/App.hpp"
#include "matrixalchemy/platform/Gl.hpp"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <imgui.h>

namespace matrixalchemy
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

    void DebugUi::draw(App &app)
    {
        ImGui::Begin("Matrix Alchemy");
        ImGui::Text("F1: toggle this panel");
        ImGui::Separator();
        ImGui::Text("Cube rotation: %.2f deg", app.cubeRotationDegrees());
        ImGui::Separator();
        ImGui::Text("Camera radius: %.2f", app.cameraRadius());
        ImGui::Text("Camera theta: %.2f deg", app.cameraThetaDegrees());
        ImGui::Text("Camera phi: %.2f deg", app.cameraPhiDegrees());
        ImGui::Separator();
        const glm::vec3 position = app.characterPosition();
        ImGui::Text("Character position: %.2f, %.2f, %.2f", position.x, position.y, position.z);
        ImGui::Text("Character rotation: %.2f deg", app.characterRotationDegrees());
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

} // namespace matrixalchemy
