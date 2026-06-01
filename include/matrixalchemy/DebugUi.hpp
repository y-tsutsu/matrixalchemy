#pragma once

struct GLFWwindow;

namespace matrixalchemy
{

    class App;

    class DebugUi
    {
    public:
        static void initialize(GLFWwindow *window);
        static void beginFrame();
        static void draw(App &app);
        static void render();
        static void shutdown();
    };

} // namespace matrixalchemy
