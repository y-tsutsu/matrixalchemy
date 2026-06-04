#include "matrixalchemy/asset/ModelPoseAnimator.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace
{

    constexpr float pi = 3.14159265358979323846F;

    float radians(float degrees)
    {
        return degrees * pi / 180.0F;
    }

    std::size_t findNodeIndex(const std::vector<matrixalchemy::asset::ModelNode> &nodes, const char *name)
    {
        for (std::size_t index = 0; index < nodes.size(); ++index)
        {
            if (nodes[index].name == name)
            {
                return index;
            }
        }

        return nodes.size();
    }

} // namespace

namespace matrixalchemy::asset
{

    void ModelPoseAnimator::initialize(const std::vector<ModelNode> &nodes)
    {
        leftArmIndex_ = findNodeIndex(nodes, "LeftArm");
        rightArmIndex_ = findNodeIndex(nodes, "RightArm");
        hasLeftArm_ = leftArmIndex_ < nodes.size();
        hasRightArm_ = rightArmIndex_ < nodes.size();
    }

    void ModelPoseAnimator::apply(float elapsedSeconds, std::vector<ModelNode> &nodes) const
    {
        const float baseArmAngle = 58.0F;
        const float spreadAngle = (std::sin(elapsedSeconds * 1.4F) * 0.5F + 0.5F) * 34.0F;
        const float armAngle = radians(baseArmAngle - spreadAngle);

        if (hasLeftArm_ && leftArmIndex_ < nodes.size())
        {
            nodes[leftArmIndex_].localTransform = nodes[leftArmIndex_].baseLocalTransform * glm::rotate(glm::mat4(1.0F), armAngle, {0.0F, 0.0F, 1.0F});
        }

        if (hasRightArm_ && rightArmIndex_ < nodes.size())
        {
            nodes[rightArmIndex_].localTransform = nodes[rightArmIndex_].baseLocalTransform * glm::rotate(glm::mat4(1.0F), -armAngle, {0.0F, 0.0F, 1.0F});
        }
    }

} // namespace matrixalchemy::asset
