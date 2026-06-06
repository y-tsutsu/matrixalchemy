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

    void VrmCharacter::updateWithInput(float deltaSeconds, const CharacterInput &input, const asset::PoseAnimationSettings &poseSettings)
    {
        controller_.update(deltaSeconds, input);
        poseSeconds_ += deltaSeconds;
        model_.applyDemoPose(poseSeconds_, poseSettings);
    }

    void VrmCharacter::draw(render::ShaderProgram &shader, const render::ToonLighting &toonLighting) const
    {
        const glm::mat4 transform = transformMatrix() * glm::scale(glm::mat4(1.0F), glm::vec3(modelScale_));
        model_.drawOutline(shader, transform, outlineWidth_, toonLighting.useMaterialOutline);
        shader.setBool("uUseToonLighting", toonLighting.enabled);
        model_.draw(shader, transform, true, &toonLighting.shadeColor, toonLighting.useMaterialShadeColor, toonLighting.useMaterialLighting);
        shader.setBool("uUseToonLighting", false);
    }

    void VrmCharacter::draw(render::ShaderProgram &shader) const
    {
        draw(shader, {});
    }

    void VrmCharacter::drawShadow(render::ShaderProgram &shader, const glm::mat4 &shadowMatrix) const
    {
        const glm::mat4 floorLift = glm::translate(glm::mat4(1.0F), {0.0F, 0.015F, 0.0F});
        const glm::mat4 transform = transformMatrix() * glm::scale(glm::mat4(1.0F), glm::vec3(modelScale_));
        model_.draw(shader, floorLift * shadowMatrix * transform, false);
    }

} // namespace matrixalchemy::scene
