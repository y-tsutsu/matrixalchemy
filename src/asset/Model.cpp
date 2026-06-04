#include "matrixalchemy/asset/Model.hpp"

#include "matrixalchemy/asset/GltfModelLoader.hpp"
#include "matrixalchemy/platform/Gl.hpp"

#include <algorithm>
#include <utility>
#include <vector>

namespace matrixalchemy::asset
{
    namespace
    {

        constexpr std::size_t maxJointMatrices = 128;

    } // namespace

    void Model::load(const std::filesystem::path &path, const glm::vec3 &color)
    {
        release();

        try
        {
            ModelData modelData = loadGltfModel(path, color);
            nodes_ = std::move(modelData.nodes);
            skins_ = std::move(modelData.skins);
            textures_ = std::move(modelData.textures);
            poseAnimator_.initialize(nodes_);

            for (ModelPrimitive &primitive : modelData.primitives)
            {
                Mesh loadedMesh;
                loadedMesh.geometry.upload(primitive.vertices, GL_TRIANGLES);
                loadedMesh.textureIndex = primitive.textureIndex;
                loadedMesh.alphaCutoff = primitive.alphaCutoff;
                loadedMesh.hasTexture = primitive.hasTexture;
                loadedMesh.alphaMask = primitive.alphaMask;
                loadedMesh.alphaBlend = primitive.alphaBlend;
                loadedMesh.doubleSided = primitive.doubleSided;
                meshes_.push_back(std::move(loadedMesh));
            }

            instances_.reserve(modelData.instances.size());
            for (const ModelInstance &instance : modelData.instances)
            {
                instances_.push_back({instance.primitiveIndex, instance.nodeIndex, instance.skinIndex, instance.transform, instance.hasSkin});
            }
        }
        catch (...)
        {
            release();
            throw;
        }
    }

    void Model::applyDemoPose(float elapsedSeconds, const PoseAnimationSettings &settings)
    {
        resetNodeLocalTransforms();
        poseAnimator_.apply(elapsedSeconds, settings, nodes_);
        updateWorldTransforms();
    }

    void Model::release()
    {
        for (Mesh &mesh : meshes_)
        {
            mesh.geometry.release();
        }
        meshes_.clear();
        instances_.clear();
        nodes_.clear();
        skins_.clear();
        textures_.clear();
    }

    void Model::draw(render::ShaderProgram &shader, const glm::mat4 &modelMatrix, bool useMaterialState) const
    {
        const bool previousCullFace = glIsEnabled(GL_CULL_FACE) == GL_TRUE;
        const bool previousBlend = glIsEnabled(GL_BLEND) == GL_TRUE;
        GLboolean previousDepthMask = GL_TRUE;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask);

        for (const MeshInstance &instance : instances_)
        {
            if (instance.meshIndex >= meshes_.size())
            {
                continue;
            }

            shader.setMat4("uModel", modelMatrix * instance.transform);
            const Mesh &mesh = meshes_[instance.meshIndex];
            const std::vector<glm::mat4> joints = jointMatrices(instance);
            shader.setBool("uUseSkinning", !joints.empty());
            shader.setMat4Array("uJointMatrices[0]", joints);

            if (useMaterialState && mesh.doubleSided)
            {
                glDisable(GL_CULL_FACE);
            }
            else if (useMaterialState)
            {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
            }

            if (useMaterialState && mesh.alphaBlend)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glDepthMask(GL_FALSE);
            }
            else if (useMaterialState)
            {
                glDisable(GL_BLEND);
                glDepthMask(previousDepthMask);
            }

            const bool useTexture = useMaterialState && mesh.hasTexture && mesh.textureIndex < textures_.size() && textures_[mesh.textureIndex].valid();
            shader.setBool("uUseTexture", useTexture);
            shader.setBool("uUseAlphaMask", useMaterialState && mesh.alphaMask);
            shader.setFloat("uAlphaCutoff", mesh.alphaCutoff);
            if (useTexture)
            {
                textures_[mesh.textureIndex].bind(0);
                shader.setInt("uBaseColorTexture", 0);
            }
            meshes_[instance.meshIndex].geometry.draw();
        }
        shader.setBool("uUseSkinning", false);
        shader.setBool("uUseTexture", false);
        shader.setBool("uUseAlphaMask", false);

        if (previousCullFace)
        {
            glEnable(GL_CULL_FACE);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }

        if (previousBlend)
        {
            glEnable(GL_BLEND);
        }
        else
        {
            glDisable(GL_BLEND);
        }
        glDepthMask(previousDepthMask);
    }

    void Model::drawOutline(render::ShaderProgram &shader, const glm::mat4 &modelMatrix, float width) const
    {
        const bool previousCullFace = glIsEnabled(GL_CULL_FACE) == GL_TRUE;
        const bool previousBlend = glIsEnabled(GL_BLEND) == GL_TRUE;

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glDisable(GL_BLEND);

        shader.setBool("uUseColorOverride", true);
        shader.setVec4("uColorOverride", {0.10F, 0.08F, 0.06F, 1.0F});
        shader.setBool("uUseTexture", false);
        shader.setBool("uUseAlphaMask", false);
        shader.setFloat("uOutlineWidth", width);

        for (const MeshInstance &instance : instances_)
        {
            if (instance.meshIndex >= meshes_.size())
            {
                continue;
            }

            shader.setMat4("uModel", modelMatrix * instance.transform);
            const std::vector<glm::mat4> joints = jointMatrices(instance);
            shader.setBool("uUseSkinning", !joints.empty());
            shader.setMat4Array("uJointMatrices[0]", joints);
            meshes_[instance.meshIndex].geometry.draw();
        }

        shader.setFloat("uOutlineWidth", 0.0F);
        shader.setBool("uUseColorOverride", false);
        shader.setBool("uUseSkinning", false);

        glCullFace(GL_BACK);
        if (previousCullFace)
        {
            glEnable(GL_CULL_FACE);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }

        if (previousBlend)
        {
            glEnable(GL_BLEND);
        }
        else
        {
            glDisable(GL_BLEND);
        }
    }

    std::vector<glm::mat4> Model::jointMatrices(const MeshInstance &instance) const
    {
        if (!instance.hasSkin || instance.skinIndex >= skins_.size())
        {
            return {};
        }

        const ModelSkin &skin = skins_[instance.skinIndex];
        const std::size_t jointCount = std::min({skin.jointNodeIndices.size(), skin.inverseBindMatrices.size(), maxJointMatrices});
        std::vector<glm::mat4> matrices;
        matrices.reserve(jointCount);

        const glm::mat4 inverseMeshTransform = glm::inverse(instance.transform);
        for (std::size_t jointIndex = 0; jointIndex < jointCount; ++jointIndex)
        {
            const std::size_t nodeIndex = skin.jointNodeIndices[jointIndex];
            if (nodeIndex >= nodes_.size())
            {
                matrices.push_back(glm::mat4(1.0F));
                continue;
            }

            matrices.push_back(inverseMeshTransform * nodes_[nodeIndex].worldTransform * skin.inverseBindMatrices[jointIndex]);
        }

        return matrices;
    }

    void Model::resetNodeLocalTransforms()
    {
        for (ModelNode &node : nodes_)
        {
            node.localTransform = node.baseLocalTransform;
        }
    }

    void Model::updateWorldTransforms()
    {
        for (ModelNode &node : nodes_)
        {
            if (node.hasParent && node.parentIndex < nodes_.size())
            {
                node.worldTransform = nodes_[node.parentIndex].worldTransform * node.localTransform;
            }
            else
            {
                node.worldTransform = node.localTransform;
            }
        }
    }

} // namespace matrixalchemy::asset
