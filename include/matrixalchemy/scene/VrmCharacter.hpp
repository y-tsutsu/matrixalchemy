#pragma once

#include "matrixalchemy/asset/Model.hpp"
#include "matrixalchemy/scene/CharacterController.hpp"
#include "matrixalchemy/scene/IDrawable.hpp"
#include "matrixalchemy/scene/IShadowCaster.hpp"

#include <filesystem>
#include <glm/glm.hpp>

namespace matrixalchemy::scene
{

    class VrmCharacter final : public IDrawable, public IShadowCaster
    {
    public:
        void load(const std::filesystem::path &path);
        void release();
        void update(float deltaSeconds, const CharacterInput &input);
        void draw(render::ShaderProgram &shader) const override;
        void drawShadow(render::ShaderProgram &shader, const glm::mat4 &shadowMatrix) const override;

        [[nodiscard]] glm::vec3 position() const { return controller_.position(); }
        [[nodiscard]] float rotationDegrees() const { return controller_.rotationDegrees(); }
        [[nodiscard]] glm::mat4 transformMatrix() const { return controller_.transformMatrix(); }
        [[nodiscard]] bool loaded() const { return !model_.empty(); }

    private:
        asset::Model model_;
        CharacterController controller_;
        float modelScale_ = 1.18F;
        float outlineWidth_ = 0.012F;
    };

} // namespace matrixalchemy::scene
