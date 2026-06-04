#pragma once

#include "matrixalchemy/asset/ModelData.hpp"

#include <cstddef>
#include <vector>

namespace matrixalchemy::asset
{

    struct PoseAnimationSettings
    {
        bool enabled = true;
        float speed = 1.8F;
        float baseArmAngleDegrees = 58.0F;
        float spreadAngleDegrees = 34.0F;
        bool headEnabled = true;
        float headYawDegrees = 8.0F;
        bool tailEnabled = true;
        float tailSwingDegrees = 18.0F;
    };

    class ModelPoseAnimator
    {
    public:
        void initialize(const std::vector<ModelNode> &nodes);
        void setHumanoidArmNodes(std::size_t leftUpperArmNodeIndex, std::size_t rightUpperArmNodeIndex);
        void setHumanoidHeadNode(std::size_t headNodeIndex);
        void apply(float elapsedSeconds, const PoseAnimationSettings &settings, std::vector<ModelNode> &nodes) const;

    private:
        std::size_t leftArmIndex_ = 0;
        std::size_t rightArmIndex_ = 0;
        std::size_t headIndex_ = 0;
        std::vector<std::size_t> tailIndices_;
        bool hasLeftArm_ = false;
        bool hasRightArm_ = false;
        bool hasHead_ = false;
    };

} // namespace matrixalchemy::asset
