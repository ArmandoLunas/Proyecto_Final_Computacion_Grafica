#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h> 

#define MAX_BONE_INFLUENCE 4

// estructura de vértice
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    // Datos de los huesos
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    float m_Weights[MAX_BONE_INFLUENCE];
};

// estructura de textura
struct Texture {
    unsigned int id;
    std::string type;
    aiString path; // Almacenamos la ruta original de assimp
};

// Información de un hueso (ID y offset matrix)
struct BoneInfo
{
    int id;
    glm::mat4 offset;
};