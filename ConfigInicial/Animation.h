#pragma once

#include <vector>
#include <string>
#include <map>

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "bone.h"
#include "Model.h"

// Clase que maneja los keyframes de un hueso
class Bone
{
public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel)
        : m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
    {
        // Copiar Keyframes de Posición
        for (int i = 0; i < channel->mNumPositionKeys; ++i)
        {
            KeyPosition data;
            data.position = glm::vec3(channel->mPositionKeys[i].mValue.x, channel->mPositionKeys[i].mValue.y, channel->mPositionKeys[i].mValue.z);
            data.timeStamp = channel->mPositionKeys[i].mTime;
            m_Positions.push_back(data);
        }
        // Copiar Keyframes de Rotación
        for (int i = 0; i < channel->mNumRotationKeys; ++i)
        {
            KeyRotation data;
            data.orientation = glm::quat(channel->mRotationKeys[i].mValue.w, channel->mRotationKeys[i].mValue.x, channel->mRotationKeys[i].mValue.y, channel->mRotationKeys[i].mValue.z);
            data.timeStamp = channel->mRotationKeys[i].mTime;
            m_Rotations.push_back(data);
        }
        // Copiar Keyframes de Escala
        for (int i = 0; i < channel->mNumScalingKeys; ++i)
        {
            KeyScale data;
            data.scale = glm::vec3(channel->mScalingKeys[i].mValue.x, channel->mScalingKeys[i].mValue.y, channel->mScalingKeys[i].mValue.z);
            data.timeStamp = channel->mScalingKeys[i].mTime;
            m_Scales.push_back(data);
        }
    }

    // Interpola y actualiza la m_LocalTransform de este hueso
    void Update(float animationTime)
    {
        glm::mat4 translation = InterpolatePosition(animationTime);
        glm::mat4 rotation = InterpolateRotation(animationTime);
        glm::mat4 scale = InterpolateScale(animationTime);
        m_LocalTransform = translation * rotation * scale;
    }

    glm::mat4 GetLocalTransform() { return m_LocalTransform; }
    std::string GetBoneName() const { return m_Name; }
    int GetBoneID() { return m_ID; }

private:
    // --- Funciones de Interpolación ---

    // Interpolar Posición
    glm::mat4 InterpolatePosition(float animationTime)
    {

        if (m_Positions.empty())
            return glm::mat4(1.0f);

        if (m_Positions.size() == 1)
            return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

        int p0Index = GetPositionIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp, m_Positions[p1Index].timeStamp, animationTime);
        glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor);

        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    // Interpolar Rotación
    glm::mat4 InterpolateRotation(float animationTime)
    {

        if (m_Rotations.empty())
            return glm::mat4(1.0f);

        if (m_Rotations.size() == 1)
            return glm::toMat4(glm::normalize(m_Rotations[0].orientation));

        int r0Index = GetRotationIndex(animationTime);
        int r1Index = r0Index + 1;
        float scaleFactor = GetScaleFactor(m_Rotations[r0Index].timeStamp, m_Rotations[r1Index].timeStamp, animationTime);
        glm::quat finalRotation = glm::slerp(m_Rotations[r0Index].orientation, m_Rotations[r1Index].orientation, scaleFactor);
        finalRotation = glm::normalize(finalRotation);

        return glm::toMat4(finalRotation);
    }

    // Interpolar Escala
    glm::mat4 InterpolateScale(float animationTime)
    {

        if (m_Scales.empty())
            return glm::mat4(1.0f);

        if (m_Scales.size() == 1)
            return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

        int s0Index = GetScaleIndex(animationTime);
        int s1Index = s0Index + 1;
        float scaleFactor = GetScaleFactor(m_Scales[s0Index].timeStamp, m_Scales[s1Index].timeStamp, animationTime);
        glm::vec3 finalScale = glm::mix(m_Scales[s0Index].scale, m_Scales[s1Index].scale, scaleFactor);

        return glm::scale(glm::mat4(1.0f), finalScale);
    }


    // --- Funciones de ayuda para encontrar keyframes ---

    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float scaleFactor = 0.0f;
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        scaleFactor = midWayLength / framesDiff;
        return scaleFactor;
    }

    int GetPositionIndex(float animationTime)
    {
        for (int index = 0; index < m_Positions.size() - 1; ++index)
            if (animationTime < m_Positions[index + 1].timeStamp) return index;
        return 0;
    }
    int GetRotationIndex(float animationTime)
    {
        for (int index = 0; index < m_Rotations.size() - 1; ++index)
            if (animationTime < m_Rotations[index + 1].timeStamp) return index;
        return 0;
    }
    int GetScaleIndex(float animationTime)
    {
        for (int index = 0; index < m_Scales.size() - 1; ++index)
            if (animationTime < m_Scales[index + 1].timeStamp) return index;
        return 0;
    }


    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;
};

// Estructura para almacenar la jerarquía de nodos de Assimp
struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

// Clase que almacena la animación completa
class Animation
{
public:
    Animation(const std::string& animationPath, Model* model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);

        // Tomamos la primera animación
        auto animation = scene->mAnimations[0];
        m_Duration = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;

        // Cargar la jerarquía de nodos
        ReadHierarchyData(m_RootNode, scene->mRootNode);

        // Cargar los huesos
        ReadMissingBones(animation, *model);
    }

    float GetTicksPerSecond() { return m_TicksPerSecond; }
    float GetDuration() { return m_Duration; }
    const AssimpNodeData& GetRootNode() { return m_RootNode; }

    // Busca un hueso por nombre
    Bone* FindBone(const std::string& name)
    {
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
            [&](const Bone& bone) { return bone.GetBoneName() == name; }
        );
        if (iter == m_Bones.end()) return nullptr;
        else return &(*iter);
    }

    std::map<std::string, BoneInfo> GetBoneInfoMap() { return m_BoneInfoMap; }


private:
    void ReadMissingBones(const aiAnimation* animation, Model& model)
    {
        int size = animation->mNumChannels;
        auto& boneInfoMap = model.m_BoneInfoMap;
        int& boneCount = model.m_BoneCounter;   

        for (int i = 0; i < size; i++)
        {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.C_Str();

            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }
            m_Bones.push_back(Bone(boneName, boneInfoMap[boneName].id, channel));
        }
        m_BoneInfoMap = boneInfoMap;
    }

    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
    {
        assert(src);
        dest.name = src->mName.C_Str();
        dest.transformation = AssimpToGLMmat4(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            ReadHierarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }

    float m_Duration;
    int m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;
};