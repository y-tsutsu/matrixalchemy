#pragma once

#include "matrixalchemy/asset/ModelData.hpp"

#include <cstddef>
#include <vector>

namespace matrixalchemy::asset
{

    class ModelPoseAnimator
    {
    public:
        void initialize(const std::vector<ModelNode> &nodes);
        void apply(float elapsedSeconds, std::vector<ModelNode> &nodes) const;

    private:
        std::size_t leftArmIndex_ = 0;
        std::size_t rightArmIndex_ = 0;
        bool hasLeftArm_ = false;
        bool hasRightArm_ = false;
    };

} // namespace matrixalchemy::asset
