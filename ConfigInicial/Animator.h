#pragma once

#include <vector>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include "Animation.h"

class Animator
{
public:
    Animator(Animation* animation)
    {
        m_CurrentTime = 0.0;
        m_CurrentAnimation = animation;

        m_FinalBoneMatrices.reserve(200);
        for (int i = 0; i < 200; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    // Avanza el tiempo de la animación
    void UpdateAnimation(float dt)
    {
        if (m_CurrentAnimation)
        {
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
            m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

    std::vector<glm::mat4> GetFinalBoneMatrices()
    {
        return m_FinalBoneMatrices;
    }

private:
    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
    {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone* bone = m_CurrentAnimation->FindBone(nodeName);

        if (bone) // Si este nodo es un hueso animado
        {
            bone->Update(m_CurrentTime); // Interpola sus keyframes
            nodeTransform = bone->GetLocalTransform(); // Obtiene su transform local
        }

        // Combina con la transformación del padre
        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        // Aplicar la offset matrix para mover el vértice
        auto boneInfoMap = m_CurrentAnimation->GetBoneInfoMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int boneIndex = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            m_FinalBoneMatrices[boneIndex] = globalTransformation * offset;
        }

        // Recorrer los hijos
        for (int i = 0; i < node->childrenCount; i++)
            CalculateBoneTransform(&node->children[i], globalTransformation);
    }


    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
};