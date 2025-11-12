#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "bone.h"
#include "Model.h" 

using namespace std;


// --- Función Helper para convertir matrices de Assimp a GLM ---
inline glm::mat4 AssimpToGLMmat4(const aiMatrix4x4& from)
{
	glm::mat4 to;
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
	return to;
}

// --- Función Helper para convertir vectores de Assimp a GLM ---
inline glm::vec3 AssimpToGLMVec3(const aiVector3D& vec)
{
	return glm::vec3(vec.x, vec.y, vec.z);
}

// --- Función Helper para convertir cuaterniones de Assimp a GLM ---
inline glm::quat AssimpToGLMQuat(const aiQuaternion& quat)
{
	return glm::quat(quat.w, quat.x, quat.y, quat.z);
}


// Representa un solo hueso o nodo animado con sus keyframes
struct Bone
{
	std::vector<aiVectorKey> m_PositionKeys;
	std::vector<aiQuatKey> m_RotationKeys;
	std::vector<aiVectorKey> m_ScalingKeys;

	glm::mat4 m_LocalTransform;
	std::string m_Name;
	int m_NumPositions;
	int m_NumRotations;
	int m_NumScalings;

	Bone(const std::string& name, aiNodeAnim* channel)
		: m_Name(name), m_LocalTransform(1.0f)
	{
		m_NumPositions = channel->mNumPositionKeys;
		for (int i = 0; i < m_NumPositions; ++i)
			m_PositionKeys.push_back(channel->mPositionKeys[i]);

		m_NumRotations = channel->mNumRotationKeys;
		for (int i = 0; i < m_NumRotations; ++i)
			m_RotationKeys.push_back(channel->mRotationKeys[i]);

		m_NumScalings = channel->mNumScalingKeys;
		for (int i = 0; i < m_NumScalings; ++i)
			m_ScalingKeys.push_back(channel->mScalingKeys[i]);
	}

	// Interpola entre keyframes
	void Update(float animationTime)
	{
		glm::mat4 translation = InterpolatePosition(animationTime);
		glm::mat4 rotation = InterpolateRotation(animationTime);
		glm::mat4 scale = InterpolateScaling(animationTime);
		m_LocalTransform = translation * rotation * scale;
	}

	// --- Funciones de Interpolación ---
	int GetPositionIndex(float animationTime)
	{
		for (int index = 0; index < m_NumPositions - 1; ++index)
			if (animationTime < m_PositionKeys[index + 1].mTime)
				return index;
		return m_NumPositions - 1; // Devolver el último índice si el tiempo se pasa
	}

	int GetRotationIndex(float animationTime)
	{
		for (int index = 0; index < m_NumRotations - 1; ++index)
			if (animationTime < m_RotationKeys[index + 1].mTime)
				return index;
		return m_NumRotations - 1; // Devolver el último índice
	}

	int GetScaleIndex(float animationTime)
	{
		for (int index = 0; index < m_NumScalings - 1; ++index)
			if (animationTime < m_ScalingKeys[index + 1].mTime)
				return index;
		return m_NumScalings - 1; // Devolver el último índice
	}

	float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
	{
		if (nextTimeStamp == lastTimeStamp) return 0.0f;

		float midWayLength = animationTime - lastTimeStamp;
		float framesDiff = nextTimeStamp - lastTimeStamp;
		float scaleFactor = midWayLength / framesDiff;
		return glm::clamp(scaleFactor, 0.0f, 1.0f); // entre 0 y 1
	}

	glm::mat4 InterpolatePosition(float animationTime)
	{
		if (m_NumPositions == 0) return glm::mat4(1.0f); // Sin claves de posición
		if (m_NumPositions == 1)
			return glm::translate(glm::mat4(1.0f), AssimpToGLMVec3(m_PositionKeys[0].mValue));

		int p0Index = GetPositionIndex(animationTime);
		int p1Index = p0Index + 1;

		// Manejar el caso de estar en o más allá del último keyframe
		if (p1Index >= m_NumPositions)
		{
			return glm::translate(glm::mat4(1.0f), AssimpToGLMVec3(m_PositionKeys[p0Index].mValue));
		}

		float scaleFactor = GetScaleFactor(m_PositionKeys[p0Index].mTime, m_PositionKeys[p1Index].mTime, animationTime);
		aiVector3D finalPosition = m_PositionKeys[p0Index].mValue + scaleFactor * (m_PositionKeys[p1Index].mValue - m_PositionKeys[p0Index].mValue);
		return glm::translate(glm::mat4(1.0f), AssimpToGLMVec3(finalPosition));
	}

	glm::mat4 InterpolateRotation(float animationTime)
	{
		if (m_NumRotations == 0) return glm::mat4(1.0f); // Sin claves de rotación
		if (m_NumRotations == 1)
		{
			return glm::mat4_cast(AssimpToGLMQuat(m_RotationKeys[0].mValue));
		}

		int p0Index = GetRotationIndex(animationTime);
		int p1Index = p0Index + 1;

		// Manejar el caso de estar en o más allá del último keyframe
		if (p1Index >= m_NumRotations)
		{
			return glm::mat4_cast(AssimpToGLMQuat(m_RotationKeys[p0Index].mValue));
		}

		float scaleFactor = GetScaleFactor(m_RotationKeys[p0Index].mTime, m_RotationKeys[p1Index].mTime, animationTime);

		aiQuaternion finalRotation;
		aiQuaternion::Interpolate(finalRotation, m_RotationKeys[p0Index].mValue, m_RotationKeys[p1Index].mValue, scaleFactor);
		finalRotation = finalRotation.Normalize();
		return glm::mat4_cast(AssimpToGLMQuat(finalRotation));
	}

	glm::mat4 InterpolateScaling(float animationTime)
	{
		if (m_NumScalings == 0) return glm::mat4(1.0f); // Sin claves de escala
		if (m_NumScalings == 1)
			return glm::scale(glm::mat4(1.0f), AssimpToGLMVec3(m_ScalingKeys[0].mValue));

		int p0Index = GetScaleIndex(animationTime);
		int p1Index = p0Index + 1;

		// Manejar el caso de estar en o más allá del último keyframe
		if (p1Index >= m_NumScalings)
		{
			return glm::scale(glm::mat4(1.0f), AssimpToGLMVec3(m_ScalingKeys[p0Index].mValue));
		}

		float scaleFactor = GetScaleFactor(m_ScalingKeys[p0Index].mTime, m_ScalingKeys[p1Index].mTime, animationTime);
		aiVector3D finalScale = m_ScalingKeys[p0Index].mValue + scaleFactor * (m_ScalingKeys[p1Index].mValue - m_ScalingKeys[p0Index].mValue);
		return glm::scale(glm::mat4(1.0f), AssimpToGLMVec3(finalScale));
	}
};

// Estructura para almacenar la jerarquía de nodos de Assimp
struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	std::vector<AssimpNodeData> children;
};


class Animation
{
public:
	float m_Duration;
	float m_TicksPerSecond;
	std::vector<Bone> m_Bones; // Contiene todos los nodos animados (huesos y no-huesos)
	AssimpNodeData m_RootNode;
	std::map<string, BoneInfo> m_BoneInfoMap; // Copia del mapa del modelo

	Animation(const std::string& animationPath, Model* model)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate | aiProcess_FlipUVs);
		if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
			return;
		}

		if (scene->mNumAnimations == 0)
		{
			cout << "ERROR::ASSIMP:: No animations found in file: " << animationPath << endl;
			return;
		}

		// Usamos solo la primera animación
		aiAnimation* animation = scene->mAnimations[0];
		m_Duration = animation->mDuration;
		m_TicksPerSecond = (animation->mTicksPerSecond != 0) ? animation->mTicksPerSecond : 25.0f;

		// Almacena el mapa de BoneInfo del modelo (que contiene las offset matrices)
		// y la jerarquía de nodos estática
		m_BoneInfoMap = model->GetBoneInfoMap();
		ReadHierarchyData(m_RootNode, scene->mRootNode);

		// Lee todos los canales de animación
		ReadMissingBones(animation, *model);

		std::cout << "[DEBUG] Nodos animados encontrados: " << m_Bones.size() << std::endl;
		for (const auto& bone : m_Bones) {
			std::cout << " > " << bone.m_Name << std::endl;
		}
		std::cout << "[DEBUG] Jerarquia de Nodos Raiz:" << std::endl;
		std::function<void(const AssimpNodeData&, int)> printNode;
		printNode = [&](const AssimpNodeData& node, int depth) {
			for (int i = 0; i < depth; ++i) std::cout << "  ";
			std::cout << "- " << node.name << std::endl;
			for (const auto& child : node.children) {
				printNode(child, depth + 1);
			}
			};
		printNode(m_RootNode, 0);
	}

	~Animation() {}

	Bone* FindBone(const std::string& name)
	{
		for (int i = 0; i < m_Bones.size(); ++i)
			if (m_Bones[i].m_Name == name)
				return &m_Bones[i];
		return nullptr;
	}

private:
	// Lee todos los canales de animación (aiNodeAnim)
	void ReadMissingBones(aiAnimation* animation, Model& model)
	{
		int size = animation->mNumChannels;

		// Almacena el mapa de BoneInfo del modelo (que contiene las offset matrices)
		// m_BoneInfoMap = model.GetBoneInfoMap();

		for (int i = 0; i < size; i++)
		{
			aiNodeAnim* channel = animation->mChannels[i];
			std::string nodeName = channel->mNodeName.data;

			m_Bones.push_back(Bone(nodeName, channel));
		}
	}

	// Copia recursivamente la jerarquía de nodos de Assimp
	void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
	{
		dest.name = src->mName.data;
		dest.transformation = AssimpToGLMmat4(src->mTransformation); // Transformación estática (bind pose)

		dest.children.resize(src->mNumChildren);
		for (int i = 0; i < src->mNumChildren; i++)
		{
			ReadHierarchyData(dest.children[i], src->mChildren[i]);
		}
	}
};