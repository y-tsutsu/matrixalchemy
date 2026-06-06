#pragma once

#include "matrixalchemy/render/ShaderProgram.hpp"

#include <glm/glm.hpp>

namespace matrixalchemy::scene
{

    /// @brief Appから毎フレーム扱えるシーンオブジェクトの基底クラス。
    ///
    /// 新しいシーンオブジェクトを追加するときはこのクラスを継承し、少なくとも
    /// draw()とrelease()を実装する。動かないオブジェクトや影を出さない
    /// オブジェクトでは、update()とdrawShadow()はoverrideしなくてよい。
    class SceneObject
    {
    public:
        /// @brief 派生クラスを基底クラスポインタ経由で破棄できるようにする。
        virtual ~SceneObject() = default;

        /// @brief 1フレーム分の状態更新を行う。
        /// @param deltaSeconds 前フレームからの経過秒数。
        virtual void update(float deltaSeconds) { static_cast<void>(deltaSeconds); }

        /// @brief 現在有効なシェーダーで自分自身を描画する。
        /// @param shader 描画に使うシェーダープログラム。
        virtual void draw(render::ShaderProgram &shader) const = 0;

        /// @brief 床へ投影する影として自分自身を描画する。
        /// @param shader 描画に使うシェーダープログラム。
        /// @param shadowMatrix ライト位置から床平面へ投影するための行列。
        virtual void drawShadow(render::ShaderProgram &shader, const glm::mat4 &shadowMatrix) const
        {
            static_cast<void>(shader);
            static_cast<void>(shadowMatrix);
        }

        /// @brief OpenGLコンテキストが有効なうちにGPUリソースを明示的に解放する。
        virtual void release() = 0;
    };

} // namespace matrixalchemy::scene
