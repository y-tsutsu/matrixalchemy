#include "matrixalchemy/scene/VrmCharacter.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace matrixalchemy::scene
{

    void VrmCharacter::load(const std::filesystem::path &path)
    {
        model_.load(path, {1.0F, 1.0F, 1.0F});
    }

    void VrmCharacter::release()
    {
        model_.release();
    }

    void VrmCharacter::update(float deltaSeconds, const CharacterInput &input)
    {
        controller_.update(deltaSeconds, input);
    }

    void VrmCharacter::draw(render::ShaderProgram &shader) const
    {
        const glm::mat4 transform = transformMatrix();
        model_.drawOutline(shader, transform, outlineWidth_);
        model_.draw(shader, transform);
    }

    void VrmCharacter::drawShadow(render::ShaderProgram &shader, const glm::mat4 &shadowMatrix) const
    {
        const glm::mat4 floorLift = glm::translate(glm::mat4(1.0F), {0.0F, 0.015F, 0.0F});
        model_.draw(shader, floorLift * shadowMatrix * transformMatrix(), false);
    }

} // namespace matrixalchemy::scene
