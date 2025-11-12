#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <string>
#include <vector>

#include <assimp/scene.h>

#define MAX_BONE_INFLUENCE 4

// estructura de vértice actualizada
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    // Datos de los huuesos
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};

// estructura de textura
struct Texture {
    unsigned int id;
    std::string type;
    aiString path;
};

// Información de un hueso (ID y offset matrix)
struct BoneInfo
{
    int id;
    glm::mat4 offset;
};

// Keyframes para un hueso
struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};