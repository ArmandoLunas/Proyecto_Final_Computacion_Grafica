#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Animation.h"

#ifndef MAX_BONES
#define MAX_BONES 200
#endif

class Animator
{
public:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;

    Animator(Animation* animation)
    {
        m_CurrentTime = 0.0;
        m_CurrentAnimation = animation;

        m_FinalBoneMatrices.reserve(MAX_BONES);
        for (int i = 0; i < MAX_BONES; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    void UpdateAnimation(float dt)
    {
        m_DeltaTime = dt;
        if (m_CurrentAnimation)
        {
            m_CurrentTime += m_CurrentAnimation->m_TicksPerSecond * dt;
            // Usamos fmod para que la animación se repita (loop)
            m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->m_Duration);
            CalculateBoneTransform(&m_CurrentAnimation->m_RootNode, glm::mat4(1.0f));
        }
    }

    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
    {
        std::string nodeName = node->name;
        // Transformación estática (bind pose) del nodo
        glm::mat4 nodeTransform = node->transformation;

        // Buscar si este nodo tiene un canal de animación
        Bone* bone = m_CurrentAnimation->FindBone(nodeName);

        if (bone)
        {
            // Si tiene animación, interpóla y usa esa transformación
            bone->Update(m_CurrentTime);
            nodeTransform = bone->m_LocalTransform;
        }

        // Multiplicamos la transformación del padre por la transformación local
        glm::mat4 globalTransform = parentTransform * nodeTransform;

        // Si este nodo es un hueso (está en el mapa de BoneInfo),
        // calculamos su matriz final y la guardamos.
        auto boneInfoMap = m_CurrentAnimation->m_BoneInfoMap;
        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int index = boneInfoMap.at(nodeName).id;
            glm::mat4 offset = boneInfoMap.at(nodeName).offset;

            // Almacenamos la transformación global completa (Root Motion)
            // multiplicada por la matriz de offset del hueso.
            m_FinalBoneMatrices[index] = globalTransform * offset;
        }

        // Repetimos para todos los hijos
        for (int i = 0; i < node->children.size(); i++)
            CalculateBoneTransform(&node->children[i], globalTransform);
    }

    std::vector<glm::mat4> GetFinalBoneMatrices()
    {
        return m_FinalBoneMatrices;
    }
};