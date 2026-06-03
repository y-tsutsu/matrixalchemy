#include "matrixalchemy/App.hpp"

#include "matrixalchemy/FileSystem.hpp"
#include "matrixalchemy/Gl.hpp"
#include "matrixalchemy/Shadow.hpp"

#if MATRIXALCHEMY_HAS_IMGUI
#include "matrixalchemy/DebugUi.hpp"
#endif

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <stdexcept>

namespace matrixalchemy
{

    App::App()
    {
        initializeWindow();
        initializeOpenGL();
        initializeScene();
    }

    App::~App()
    {
#if MATRIXALCHEMY_HAS_IMGUI
        DebugUi::shutdown();
#endif
        character_.release();
        sampleModel_.release();
        cube_.release();
        axisGizmo_.release();
        gridFloor_.release();
        shader_.release();
        if (window_ != nullptr)
        {
            glfwDestroyWindow(window_);
        }
        glfwTerminate();
    }

    int App::run()
    {
        auto previousTime = std::chrono::steady_clock::now();

        while (glfwWindowShouldClose(window_) == 0)
        {
            const auto currentTime = std::chrono::steady_clock::now();
            const std::chrono::duration<float> elapsed = currentTime - previousTime;
            previousTime = currentTime;

            processInput();
            update(elapsed.count());
            render();

            glfwSwapBuffers(window_);
            glfwPollEvents();
        }

        return 0;
    }

    void App::framebufferSizeCallback(GLFWwindow *window, int width, int height)
    {
        auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
        if (app != nullptr)
        {
            app->resize(width, height);
        }
    }

    void App::keyCallback(GLFWwindow *window, int key, int, int action, int)
    {
        auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
        if (app == nullptr || action != GLFW_PRESS)
        {
            return;
        }

        if (key == GLFW_KEY_ESCAPE)
        {
            app->requestClose();
        }
        else if (key == GLFW_KEY_F1)
        {
            app->showDebugUi_ = !app->showDebugUi_;
        }
    }

    void App::cursorPositionCallback(GLFWwindow *window, double x, double y)
    {
        auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
        if (app != nullptr)
        {
            app->camera_.drag(x, y);
        }
    }

    void App::mouseButtonCallback(GLFWwindow *window, int button, int action, int)
    {
        auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
        if (app == nullptr || button != GLFW_MOUSE_BUTTON_LEFT)
        {
            return;
        }

        if (action == GLFW_PRESS)
        {
            double x = 0.0;
            double y = 0.0;
            glfwGetCursorPos(window, &x, &y);
            app->camera_.beginDrag(x, y);
        }
        else if (action == GLFW_RELEASE)
        {
            app->camera_.endDrag();
        }
    }

    void App::scrollCallback(GLFWwindow *window, double, double yOffset)
    {
        auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
        if (app != nullptr)
        {
            app->camera_.zoom(yOffset);
        }
    }

    void App::initializeWindow()
    {
        if (glfwInit() == GLFW_FALSE)
        {
            throw std::runtime_error("Failed to initialize GLFW.");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_STENCIL_BITS, 8);

        window_ = glfwCreateWindow(width_, height_, "Matrix Alchemy", nullptr, nullptr);
        if (window_ == nullptr)
        {
            throw std::runtime_error("Failed to create the GLFW window.");
        }

        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
        glfwSetKeyCallback(window_, keyCallback);
        glfwSetCursorPosCallback(window_, cursorPositionCallback);
        glfwSetMouseButtonCallback(window_, mouseButtonCallback);
        glfwSetScrollCallback(window_, scrollCallback);
        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1);
    }

    void App::initializeOpenGL()
    {
#if MATRIXALCHEMY_GLAD2
        if (gladLoadGL(glfwGetProcAddress) == 0)
        {
            throw std::runtime_error("Failed to initialize GLAD.");
        }
#elif MATRIXALCHEMY_GLAD1
        if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0)
        {
            throw std::runtime_error("Failed to initialize GLAD.");
        }
#endif

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.62F, 0.70F, 0.80F, 1.0F);

#if MATRIXALCHEMY_HAS_IMGUI
        DebugUi::initialize(window_);
#endif
    }

    void App::initializeScene()
    {
        const std::string vertexShaderSource = readTextFile(resolveAssetPath("assets/shaders/color.vert"));
        const std::string fragmentShaderSource = readTextFile(resolveAssetPath("assets/shaders/color.frag"));
        shader_.create(vertexShaderSource, fragmentShaderSource);

        gridFloor_.create(5.0F, 10);
        axisGizmo_.create(5.0F);
        cube_.create(1.0F);
        character_.create();

        const std::filesystem::path saurusPath = std::filesystem::path(MATRIXALCHEMY_SOURCE_DIR) / "assets/models/saurus.vrm";
        if (std::filesystem::exists(saurusPath))
        {
            sampleModel_.load(saurusPath, {0.90F, 0.45F, 0.10F});
        }
        else
        {
            sampleModel_.load(resolveAssetPath("assets/models/simple-triangle.gltf"), {0.90F, 0.45F, 0.10F});
        }
    }

    void App::processInput()
    {
        characterInput_.moveForward = glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS;
        characterInput_.moveBackward = glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS;
        characterInput_.turnLeft = glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_PRESS;
        characterInput_.turnRight = glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS;
    }

    void App::update(float deltaSeconds)
    {
        cube_.update(deltaSeconds);
        character_.update(deltaSeconds, characterInput_);
    }

    void App::render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        const float aspect = static_cast<float>(width_) / static_cast<float>(height_);
        const glm::mat4 projection = glm::perspective(glm::radians(60.0F), aspect, 0.1F, 100.0F);
        const glm::mat4 view = camera_.viewMatrix();
        const glm::mat4 sampleModelMatrix = glm::translate(glm::mat4(1.0F), {2.0F, 0.0F, 1.5F});

        shader_.use();
        shader_.setMat4("uProjection", projection);
        shader_.setMat4("uView", view);
        shader_.setBool("uUseColorOverride", false);
        shader_.setBool("uUseTexture", false);
        shader_.setBool("uUseAlphaMask", false);

        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        gridFloor_.draw(shader_);
        glStencilMask(0x00);

        const glm::mat4 shadowMatrix = planarShadowMatrix({0.0F, 1.0F, 0.0F, 0.0F}, {-3.0F, 6.0F, -4.0F, 1.0F});
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        shader_.setBool("uUseColorOverride", true);
        shader_.setVec4("uColorOverride", {0.0F, 0.0F, 0.0F, 0.35F});
        cube_.drawShadow(shader_, shadowMatrix);
        character_.drawShadow(shader_, shadowMatrix);
        sampleModel_.draw(shader_, shadowMatrix * sampleModelMatrix, false);
        shader_.setBool("uUseColorOverride", false);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glStencilMask(0xFF);
        glDisable(GL_STENCIL_TEST);

        axisGizmo_.draw(shader_);
        cube_.draw(shader_);
        character_.draw(shader_);
        sampleModel_.draw(shader_, sampleModelMatrix);

#if MATRIXALCHEMY_HAS_IMGUI
        if (showDebugUi_)
        {
            DebugUi::beginFrame();
            DebugUi::draw(*this);
            DebugUi::render();
        }
#endif
    }

    void App::resize(int width, int height)
    {
        width_ = std::max(width, 1);
        height_ = std::max(height, 1);
        glViewport(0, 0, width_, height_);
    }

    void App::requestClose()
    {
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }

} // namespace matrixalchemy
