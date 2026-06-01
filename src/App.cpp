#include "matrixalchemy/App.hpp"

#include "matrixalchemy/FileSystem.hpp"
#include "matrixalchemy/Gl.hpp"

#if MATRIXALCHEMY_HAS_IMGUI
#include "matrixalchemy/DebugUi.hpp"
#endif

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <stdexcept>

namespace
{

    float radians(float degrees)
    {
        return degrees * 3.14159265358979323846F / 180.0F;
    }

} // namespace

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

    void App::initializeWindow()
    {
        if (glfwInit() == GLFW_FALSE)
        {
            throw std::runtime_error("Failed to initialize GLFW.");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window_ = glfwCreateWindow(width_, height_, "Matrix Alchemy", nullptr, nullptr);
        if (window_ == nullptr)
        {
            throw std::runtime_error("Failed to create the GLFW window.");
        }

        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
        glfwSetKeyCallback(window_, keyCallback);
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
    }

    void App::processInput()
    {
        if (glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            cameraTheta_ -= 1.0F;
        }
        if (glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            cameraTheta_ += 1.0F;
        }
        if (glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS)
        {
            cameraPhi_ = std::min(cameraPhi_ + 1.0F, 85.0F);
        }
        if (glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            cameraPhi_ = std::max(cameraPhi_ - 1.0F, -85.0F);
        }
    }

    void App::update(float deltaSeconds)
    {
        cube_.update(deltaSeconds);
    }

    void App::render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float aspect = static_cast<float>(width_) / static_cast<float>(height_);
        const glm::mat4 projection = glm::perspective(radians(60.0F), aspect, 0.1F, 100.0F);

        const float theta = radians(cameraTheta_);
        const float phi = radians(cameraPhi_);
        const glm::vec3 cameraPosition = {
            cameraRadius_ * std::cos(theta) * std::cos(phi),
            cameraRadius_ * std::sin(phi),
            cameraRadius_ * std::sin(theta) * std::cos(phi),
        };
        const glm::mat4 view = glm::lookAt(cameraPosition, cameraTarget_, {0.0F, 1.0F, 0.0F});
        shader_.use();
        shader_.setMat4("uProjection", projection);
        shader_.setMat4("uView", view);

        gridFloor_.draw(shader_);
        axisGizmo_.draw(shader_);
        cube_.draw(shader_);

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
