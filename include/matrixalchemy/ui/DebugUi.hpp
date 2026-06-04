#pragma once

struct GLFWwindow;

namespace matrixalchemy::app
{
    class App;
}

namespace matrixalchemy::ui
{

    class DebugUi
    {
    public:
        static void initialize(GLFWwindow *window);
        static void beginFrame();
        static void draw(app::App &app);
        static void render();
        static bool wantsKeyboardInput();
        static bool wantsMouseInput();
        static void shutdown();
    };

} // namespace matrixalchemy::ui
