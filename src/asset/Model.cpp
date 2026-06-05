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
            if (modelData.leftUpperArmNodeIndex.has_value() && modelData.rightUpperArmNodeIndex.has_value())
            {
                poseAnimator_.setHumanoidArmNodes(*modelData.leftUpperArmNodeIndex, *modelData.rightUpperArmNodeIndex);
            }
            if (modelData.headNodeIndex.has_value())
            {
                poseAnimator_.setHumanoidHeadNode(*modelData.headNodeIndex);
            }

            for (ModelPrimitive &primitive : modelData.primitives)
            {
                Mesh loadedMesh;
                loadedMesh.geometry.upload(primitive.vertices, GL_TRIANGLES);
                loadedMesh.textureIndex = primitive.textureIndex;
                loadedMesh.alphaCutoff = primitive.alphaCutoff;
                loadedMesh.toonShadeColor = primitive.toonShadeColor;
                loadedMesh.toonRimColor = primitive.toonRimColor;
                loadedMesh.toonEmissionColor = primitive.toonEmissionColor;
                loadedMesh.toonShadeShift = primitive.toonShadeShift;
                loadedMesh.toonShadeToony = primitive.toonShadeToony;
                loadedMesh.toonRimFresnelPower = primitive.toonRimFresnelPower;
                loadedMesh.toonOutlineColor = primitive.toonOutlineColor;
                loadedMesh.toonOutlineWidth = primitive.toonOutlineWidth;
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

    void Model::draw(render::ShaderProgram &shader,
                     const glm::mat4 &modelMatrix,
                     bool useMaterialState,
                     const glm::vec3 *toonShadeColor,
                     bool useMaterialToonShadeColor) const
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
            if (useMaterialState && useMaterialToonShadeColor && mesh.toonShadeColor.has_value())
            {
                shader.setVec3("uToonShadeColor", *mesh.toonShadeColor);
            }
            else if (useMaterialState && toonShadeColor != nullptr)
            {
                shader.setVec3("uToonShadeColor", *toonShadeColor);
            }
            shader.setVec3("uToonRimColor", useMaterialState ? mesh.toonRimColor.value_or(glm::vec3(0.0F)) : glm::vec3(0.0F));
            shader.setVec3("uToonEmissionColor", useMaterialState ? mesh.toonEmissionColor.value_or(glm::vec3(0.0F)) : glm::vec3(0.0F));
            shader.setFloat("uToonShadeShift", useMaterialState ? mesh.toonShadeShift.value_or(0.0F) : 0.0F);
            shader.setFloat("uToonShadeToony", useMaterialState ? mesh.toonShadeToony.value_or(0.0F) : 0.0F);
            shader.setFloat("uToonRimPower", useMaterialState ? mesh.toonRimFresnelPower.value_or(2.5F) : 2.5F);
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

    void Model::drawOutline(render::ShaderProgram &shader, const glm::mat4 &modelMatrix, float fallbackWidth) const
    {
        const bool previousCullFace = glIsEnabled(GL_CULL_FACE) == GL_TRUE;
        const bool previousBlend = glIsEnabled(GL_BLEND) == GL_TRUE;

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glDisable(GL_BLEND);

        shader.setBool("uUseColorOverride", true);
        shader.setBool("uUseTexture", false);
        shader.setBool("uUseAlphaMask", false);

        for (const MeshInstance &instance : instances_)
        {
            if (instance.meshIndex >= meshes_.size())
            {
                continue;
            }

            shader.setMat4("uModel", modelMatrix * instance.transform);
            const Mesh &mesh = meshes_[instance.meshIndex];
            const float width = mesh.toonOutlineWidth.value_or(fallbackWidth);
            if (width <= 0.0F)
            {
                continue;
            }
            shader.setVec4("uColorOverride", mesh.toonOutlineColor.value_or(glm::vec4(0.10F, 0.08F, 0.06F, 1.0F)));
            shader.setFloat("uOutlineWidth", width);
            const std::vector<glm::mat4> joints = jointMatrices(instance);
            shader.setBool("uUseSkinning", !joints.empty());
            shader.setMat4Array("uJointMatrices[0]", joints);
            mesh.geometry.draw();
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

        // glTFのjointはモデル全体のノード空間で評価されるので、メッシュインスタンスの
        // ローカル空間へ戻してからinverse bind matrixを掛ける。
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
