#include "matrixalchemy/app/App.hpp"

#include "matrixalchemy/platform/FileSystem.hpp"
#include "matrixalchemy/platform/Gl.hpp"
#include "matrixalchemy/render/ShaderSources.hpp"
#include "matrixalchemy/render/Shadow.hpp"

#if MATRIXALCHEMY_HAS_IMGUI
#include "matrixalchemy/ui/DebugUi.hpp"
#endif

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <optional>
#include <stdexcept>

namespace matrixalchemy::app
{
    namespace
    {

        constexpr float floorHalfSize = 5.0F;
        constexpr int floorTileCount = 10;
        constexpr float axisLength = 6.5F;
        constexpr float lightMarkerRadius = 0.085F;
        constexpr int lightMarkerSlices = 16;
        constexpr int lightMarkerStacks = 8;
        constexpr glm::vec3 initialLightPosition = {-4.33F, 5.6F, -3.04F};

    } // namespace

    /// @brief アプリケーションに必要なウィンドウ、OpenGL、シーンを初期化する。
    App::App()
    {
        initializeWindow();
        initializeOpenGL();
        initializeScene();
    }

    /// @brief OpenGLリソース、Debug UI、GLFWウィンドウを終了順に解放する。
    App::~App()
    {
#if MATRIXALCHEMY_HAS_IMGUI
        ui::DebugUi::shutdown();
#endif
        releaseSceneResources();
        shader_.release();
        if (window_ != nullptr)
        {
            glfwDestroyWindow(window_);
        }
        glfwTerminate();
    }

    /// @brief メインループを実行する。
    /// @return アプリケーションの終了コード。
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

    /// @brief 現在有効なキャラクターのワールド座標を取得する。
    /// @return VRMキャラクター、またはフォールバックキャラクターの座標。
    glm::vec3 App::characterPosition() const
    {
        return useVrmCharacter_ ? vrmCharacter_.position() : fallbackCharacter_.position();
    }

    /// @brief 現在有効なキャラクターのY軸回転角を取得する。
    /// @return キャラクターの回転角度。単位は度。
    float App::characterRotationDegrees() const
    {
        return useVrmCharacter_ ? vrmCharacter_.rotationDegrees() : fallbackCharacter_.rotationDegrees();
    }

    /// @brief 現在有効なキャラクターの描画上の高さを取得する。
    /// @return Debug UIに表示するキャラクターの高さ。
    float App::characterRenderHeight() const
    {
        return useVrmCharacter_ ? vrmCharacter_.renderHeight() : fallbackCharacter_.renderHeight();
    }

    /// @brief GLFWのフレームバッファサイズ変更通知をAppのリサイズ処理へ橋渡しする。
    /// @param window コールバックを発生させたGLFWウィンドウ。
    /// @param width 新しいフレームバッファ幅。
    /// @param height 新しいフレームバッファ高さ。
    void App::framebufferSizeCallback(GLFWwindow *window, int width, int height)
    {
        auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
        if (app != nullptr)
        {
            app->resize(width, height);
        }
    }

    /// @brief GLFWのキー入力通知をAppのショートカット処理へ橋渡しする。
    /// @param window コールバックを発生させたGLFWウィンドウ。
    /// @param key 入力されたGLFWキーコード。
    /// @param action キー操作の種類。
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

    /// @brief GLFWのカーソル移動通知をカメラドラッグへ橋渡しする。
    /// @param window コールバックを発生させたGLFWウィンドウ。
    /// @param x カーソルのX座標。
    /// @param y カーソルのY座標。
    void App::cursorPositionCallback(GLFWwindow *window, double x, double y)
    {
        auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
        if (app != nullptr)
        {
#if MATRIXALCHEMY_HAS_IMGUI
            if (app->showDebugUi_ && ui::DebugUi::wantsMouseInput())
            {
                return;
            }
#endif
            app->camera_.drag(x, y);
        }
    }

    /// @brief GLFWのマウスボタン通知をカメラドラッグの開始・終了へ橋渡しする。
    /// @param window コールバックを発生させたGLFWウィンドウ。
    /// @param button 入力されたGLFWマウスボタン。
    /// @param action ボタン操作の種類。
    void App::mouseButtonCallback(GLFWwindow *window, int button, int action, int)
    {
        auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
        if (app == nullptr || button != GLFW_MOUSE_BUTTON_LEFT)
        {
            return;
        }

#if MATRIXALCHEMY_HAS_IMGUI
        if (app->showDebugUi_ && ui::DebugUi::wantsMouseInput())
        {
            app->camera_.endDrag();
            return;
        }
#endif

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

    /// @brief GLFWのスクロール通知をカメラズームへ橋渡しする。
    /// @param window コールバックを発生させたGLFWウィンドウ。
    /// @param yOffset 縦方向のスクロール量。
    void App::scrollCallback(GLFWwindow *window, double, double yOffset)
    {
        auto *app = static_cast<App *>(glfwGetWindowUserPointer(window));
        if (app != nullptr)
        {
#if MATRIXALCHEMY_HAS_IMGUI
            if (app->showDebugUi_ && ui::DebugUi::wantsMouseInput())
            {
                return;
            }
#endif
            app->camera_.zoom(yOffset);
        }
    }

    /// @brief GLFWを初期化し、OpenGLコンテキストを持つウィンドウを作成する。
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

    /// @brief OpenGL関数ローダーと初期レンダーステートを設定する。
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
        ui::DebugUi::initialize(window_);
#endif
    }

    /// @brief シェーダーとシーンオブジェクトを作成し、実行時モデルを読み込む。
    void App::initializeScene()
    {
        shader_.create(render::shader_sources::colorVertex, render::shader_sources::colorFragment);

        gridFloor_.create(floorHalfSize, floorTileCount);
        arcaneRing_.create(1.55F, 128);
        axisGizmo_.create(axisLength);
        lightMarker_.create(lightMarkerRadius, lightMarkerSlices, lightMarkerStacks);
        lightMarker_.setPosition(initialLightPosition);
        floatingCubes_.create(1.0F);
        fallbackCharacter_.create();

        const std::optional<std::filesystem::path> saurusPath = platform::findRuntimeAssetPath("saurus.vrm");
        if (saurusPath.has_value())
        {
            vrmCharacter_.load(*saurusPath);
            useVrmCharacter_ = true;
        }
        else
        {
            useVrmCharacter_ = false;
        }

        registerSceneObjects();
    }

    /// @brief 毎フレーム共通処理する通常のシーンオブジェクトを描画順に登録する。
    void App::registerSceneObjects()
    {
        sceneObjects_ = {
            &arcaneRing_,
            &lightMarker_,
            &floatingCubes_,
        };

        if (useVrmCharacter_)
        {
            activeCharacter_ = &vrmCharacter_;
        }
        else
        {
            activeCharacter_ = &fallbackCharacter_;
        }
    }

    /// @brief Appが所有するシーンオブジェクトのOpenGLリソースを明示的に解放する。
    void App::releaseSceneResources()
    {
        // 床はステンシル制御、デバッグ軸はF1表示、キャラクターはMToon設定が絡むので通常sceneリストから外している。
        axisGizmo_.release();
        fallbackCharacter_.release();
        vrmCharacter_.release();
        gridFloor_.release();

        for (auto iterator = sceneObjects_.rbegin(); iterator != sceneObjects_.rend(); ++iterator)
        {
            (*iterator)->release();
        }
    }

    /// @brief キーボード状態を読み取り、キャラクター入力へ反映する。
    void App::processInput()
    {
#if MATRIXALCHEMY_HAS_IMGUI
        if (showDebugUi_ && ui::DebugUi::wantsKeyboardInput())
        {
            characterInput_ = {};
            return;
        }
#endif
        characterInput_.moveForward = glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS;
        characterInput_.moveBackward = glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS;
        characterInput_.turnLeft = glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_PRESS;
        characterInput_.turnRight = glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS;
    }

    /// @brief 1フレーム分のシーン状態を更新する。
    /// @param deltaSeconds 前フレームからの経過秒数。
    void App::update(float deltaSeconds)
    {
        updateRegisteredSceneObjects(deltaSeconds);
        arcaneRing_.setCenter(characterPosition());
        if (useVrmCharacter_)
        {
            vrmCharacter_.updateWithInput(deltaSeconds, characterInput_, poseAnimationSettings_);
        }
        else
        {
            fallbackCharacter_.updateWithInput(deltaSeconds, characterInput_);
        }
    }

    /// @brief 1フレーム分の描画処理を実行する。
    void App::render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        const float aspect = static_cast<float>(width_) / static_cast<float>(height_);
        const glm::mat4 projection = glm::perspective(glm::radians(60.0F), aspect, 0.1F, 100.0F);
        const glm::mat4 view = camera_.viewMatrix();

        setupFrameShader(projection, view);
        drawFloorMask();
        drawProjectedShadows();
        drawRegisteredSceneObjects();
        drawActiveCharacter();
        drawDebugUi();
    }

    /// @brief 登録済みシーンオブジェクトの共通更新処理を呼び出す。
    /// @param deltaSeconds 前フレームからの経過秒数。
    void App::updateRegisteredSceneObjects(float deltaSeconds)
    {
        for (scene::SceneObject *object : sceneObjects_)
        {
            object->update(deltaSeconds);
        }
    }

    /// @brief フレーム描画の開始時に共通シェーダー状態を設定する。
    /// @param projection 射影行列。
    /// @param view ビュー行列。
    void App::setupFrameShader(const glm::mat4 &projection, const glm::mat4 &view)
    {
        shader_.use();
        shader_.setMat4("uProjection", projection);
        shader_.setMat4("uView", view);
        shader_.setBool("uUseColorOverride", false);
        shader_.setBool("uUseTexture", false);
        shader_.setBool("uUseAlphaMask", false);
        shader_.setBool("uUseSkinning", false);
        shader_.setBool("uUseToonLighting", false);
        shader_.setFloat("uOutlineWidth", 0.0F);
        shader_.setVec3("uCameraPosition", camera_.position());
    }

    /// @brief 床を描画し、影を制限するためのステンシルマスクを作成する。
    void App::drawFloorMask()
    {
        // 床だけに影を出したいので、まず床を描きながらステンシルに床領域を記録する。
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        gridFloor_.draw(shader_);
        glStencilMask(0x00);
    }

    /// @brief ライト位置から床へ投影した平面影を描画する。
    void App::drawProjectedShadows()
    {
        // 影の形はライト位置から床平面へ頂点を投影する行列で作る。
        const glm::vec4 lightPosition = {lightMarker_.position(), 1.0F};
        shader_.setVec3("uLightPosition", lightMarker_.position());
        shader_.setVec3("uToonShadeColor", toonLighting_.shadeColor);
        shader_.setFloat("uToonThreshold", toonLighting_.threshold);
        shader_.setFloat("uToonSoftness", toonLighting_.softness);
        shader_.setFloat("uToonLitStrength", toonLighting_.litStrength);
        shader_.setVec3("uToonRimColor", {0.0F, 0.0F, 0.0F});
        shader_.setVec3("uToonEmissionColor", {0.0F, 0.0F, 0.0F});
        shader_.setFloat("uToonShadeShift", 0.0F);
        shader_.setFloat("uToonShadeToony", 0.0F);
        shader_.setFloat("uToonRimPower", 2.5F);
        const glm::mat4 shadowMatrix = render::planarShadowMatrix({0.0F, 1.0F, 0.0F, 0.0F}, lightPosition);
        // ステンシルで床領域だけを通し、影は半透明で重ねる。深度は書かない。
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilMask(0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        shader_.setBool("uUseColorOverride", true);
        shader_.setVec4("uColorOverride", {0.0F, 0.0F, 0.0F, 0.35F});
        for (const scene::SceneObject *object : sceneObjects_)
        {
            object->drawShadow(shader_, shadowMatrix);
        }
        if (activeCharacter_ != nullptr)
        {
            activeCharacter_->drawShadow(shader_, shadowMatrix);
        }
        shader_.setBool("uUseColorOverride", false);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glStencilMask(0xFF);
        glDisable(GL_STENCIL_TEST);
    }

    /// @brief 登録済みシーンオブジェクトとデバッグ軸を描画する。
    void App::drawRegisteredSceneObjects()
    {
        if (showDebugUi_)
        {
            // デバッグ軸はF1で表示を切り替える補助表示なので、通常のsceneObjects_には登録しない。
            axisGizmo_.draw(shader_);
        }

        for (const scene::SceneObject *object : sceneObjects_)
        {
            object->draw(shader_);
        }
    }

    /// @brief 現在有効なキャラクターを描画する。
    void App::drawActiveCharacter()
    {
        if (useVrmCharacter_)
        {
            vrmCharacter_.draw(shader_, toonLighting_);
        }
        else if (activeCharacter_ != nullptr)
        {
            activeCharacter_->draw(shader_);
        }
    }

    /// @brief Debug UIが有効な場合にDear ImGuiのフレームを描画する。
    void App::drawDebugUi()
    {
#if MATRIXALCHEMY_HAS_IMGUI
        if (showDebugUi_)
        {
            ui::DebugUi::beginFrame();
            ui::DebugUi::draw(*this);
            ui::DebugUi::render();
        }
#endif
    }

    /// @brief フレームバッファサイズに合わせてビューポートを更新する。
    /// @param width 新しい幅。
    /// @param height 新しい高さ。
    void App::resize(int width, int height)
    {
        width_ = std::max(width, 1);
        height_ = std::max(height, 1);
        glViewport(0, 0, width_, height_);
    }

    /// @brief GLFWウィンドウへ終了要求を設定する。
    void App::requestClose()
    {
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }

} // namespace matrixalchemy::app
