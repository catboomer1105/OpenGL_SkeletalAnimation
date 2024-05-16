#include "MySkeletalAnimation/animation.h"

#include "MySkeletalAnimation/assimp_glm_helpers.h"

Animation::Animation(const std::string &animationPath, Model *model)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    auto animation = scene->mAnimations[0];
    auto animationCount = scene->mNumAnimations;
    std::cout << "Animation Count: " << animationCount << std::endl;
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
    globalInverseTransform = AssimpGLMHelpers::ConvertMatrixToGLMFormat(globalTransformation.Inverse());
    ReadHeirarchyData(m_RootAssimpDataNode, scene->mRootNode);
    ReadMissingBones(animation, *model);
}

void Animation::ReadHeirarchyData(AssimpNodeData &dest, const aiNode *src)
{
    assert(src);

    dest.name = src->mName.data;
    dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++)
    {
        AssimpNodeData newData;
        ReadHeirarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

void Animation::ReadMissingBones(const aiAnimation *animation, Model &model)
{
    int size = animation->mNumChannels;

    auto &boneInfoMap = model.GetBoneInfoMap(); // getting m_BoneInfoMap from Model class
    int &boneCount = model.GetBoneCount();      // getting the m_BoneCounter from Model class

    // reading channels(bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            boneInfoMap[boneName].bone_id = boneCount;
            boneCount++;

        }
        m_Bones.push_back(Bone(boneName, boneInfoMap[boneName].bone_id, channel));
    }

    m_BoneInfoMap = boneInfoMap;
}
