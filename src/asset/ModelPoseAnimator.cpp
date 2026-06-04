#include "matrixalchemy/asset/ModelPoseAnimator.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <string>

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

    std::vector<std::size_t> findTailNodes(const std::vector<matrixalchemy::asset::ModelNode> &nodes)
    {
        std::vector<std::size_t> indices;
        for (std::size_t index = 0; index < nodes.size(); ++index)
        {
            if (nodes[index].name.starts_with("tail"))
            {
                indices.push_back(index);
            }
        }
        return indices;
    }

} // namespace

namespace matrixalchemy::asset
{

    void ModelPoseAnimator::initialize(const std::vector<ModelNode> &nodes)
    {
        leftArmIndex_ = findNodeIndex(nodes, "LeftArm");
        rightArmIndex_ = findNodeIndex(nodes, "RightArm");
        headIndex_ = findNodeIndex(nodes, "Head");
        tailIndices_ = findTailNodes(nodes);
        hasLeftArm_ = leftArmIndex_ < nodes.size();
        hasRightArm_ = rightArmIndex_ < nodes.size();
        hasHead_ = headIndex_ < nodes.size();
    }

    void ModelPoseAnimator::setHumanoidArmNodes(std::size_t leftUpperArmNodeIndex, std::size_t rightUpperArmNodeIndex)
    {
        leftArmIndex_ = leftUpperArmNodeIndex;
        rightArmIndex_ = rightUpperArmNodeIndex;
        hasLeftArm_ = true;
        hasRightArm_ = true;
    }

    void ModelPoseAnimator::setHumanoidHeadNode(std::size_t headNodeIndex)
    {
        headIndex_ = headNodeIndex;
        hasHead_ = true;
    }

    void ModelPoseAnimator::apply(float elapsedSeconds, const PoseAnimationSettings &settings, std::vector<ModelNode> &nodes) const
    {
        // クリップ再生ではなく、ノードのローカル変換を直接少しだけ変えてスキニングの効果を見る。
        const float spreadRatio = settings.enabled ? std::sin(elapsedSeconds * settings.speed) * 0.5F + 0.5F : 0.0F;
        const float spreadAngle = spreadRatio * settings.spreadAngleDegrees;
        const float armAngle = radians(settings.baseArmAngleDegrees - spreadAngle);

        if (hasLeftArm_ && leftArmIndex_ < nodes.size())
        {
            nodes[leftArmIndex_].localTransform = nodes[leftArmIndex_].baseLocalTransform * glm::rotate(glm::mat4(1.0F), armAngle, {0.0F, 0.0F, 1.0F});
        }

        if (hasRightArm_ && rightArmIndex_ < nodes.size())
        {
            nodes[rightArmIndex_].localTransform = nodes[rightArmIndex_].baseLocalTransform * glm::rotate(glm::mat4(1.0F), -armAngle, {0.0F, 0.0F, 1.0F});
        }

        if (settings.headEnabled && hasHead_ && headIndex_ < nodes.size())
        {
            const float yaw = radians(std::sin(elapsedSeconds * settings.speed * 0.8F) * settings.headYawDegrees);
            nodes[headIndex_].localTransform = nodes[headIndex_].baseLocalTransform * glm::rotate(glm::mat4(1.0F), yaw, {0.0F, 1.0F, 0.0F});
        }

        if (settings.tailEnabled)
        {
            for (std::size_t index = 0; index < tailIndices_.size(); ++index)
            {
                const std::size_t nodeIndex = tailIndices_[index];
                if (nodeIndex >= nodes.size())
                {
                    continue;
                }

                // 先端へ行くほど位相と振れ幅を変えて、硬い一本の棒に見えないようにする。
                const float phase = static_cast<float>(index) * 0.55F;
                const float swingScale = 1.0F - static_cast<float>(index) * 0.14F;
                const float yaw = radians(std::sin(elapsedSeconds * settings.speed * 1.25F + phase) * settings.tailSwingDegrees * std::max(swingScale, 0.35F));
                nodes[nodeIndex].localTransform = nodes[nodeIndex].baseLocalTransform * glm::rotate(glm::mat4(1.0F), yaw, {0.0F, 1.0F, 0.0F});
            }
        }
    }

} // namespace matrixalchemy::asset
