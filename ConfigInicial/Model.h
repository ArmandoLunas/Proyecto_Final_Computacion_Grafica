#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SOIL2/SOIL2.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"  
#include "Shader.h"

using namespace std;

glm::mat4 AssimpToGLMmat4(const aiMatrix4x4& from);

GLint TextureFromFile(const char* path, string directory);


class Model
{
public:
	/* Datos del Modelo  */
	vector<Texture> textures_loaded;	// Almacena texturas ya cargadas
	vector<Mesh> meshes;
	string directory;

	// --- Datos para Animación Esquelética ---
	map<string, BoneInfo> m_BoneInfoMap;
	int m_BoneCounter = 0;               // Contador de huesos

	Model(GLchar* path)
	{
		this->loadModel(path);
	}

	void Draw(Shader shader)
	{
		for (GLuint i = 0; i < this->meshes.size(); i++)
		{
			this->meshes[i].Draw(shader);
		}
	}

	// --- Getters públicos para el mapa de huesos ---
	const map<string, BoneInfo>& GetBoneInfoMap() const { return m_BoneInfoMap; }
	int& GetBoneCount() { return m_BoneCounter; }

private:

	// Carga el modelo usando ASSIMP
	void loadModel(string path)
	{
		// Leer archivo vía ASSIMP
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(path,
			aiProcess_Triangulate |
			aiProcess_FlipUVs |
			aiProcess_GenSmoothNormals);

		// Checar errores
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// Obtener el directorio base
		this->directory = path.substr(0, path.find_last_of('/'));

		// Procesar el nodo raíz recursivamente
		this->processNode(scene->mRootNode, scene);
	}

	// Procesa un nodo
	void processNode(aiNode* node, const aiScene* scene)
	{
		// Procesar todas las mallas del nodo
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->meshes.push_back(this->processMesh(mesh, scene));
		}

		// Continuar recursivamente con los hijos
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene);
		}
	}

	// --- Función para añadir datos de hueso a un vértice ---
	void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
	{
		// Busca un slot vacío en el vértice para añadir la info del hueso
		for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
		{
			if (vertex.m_BoneIDs[i] < 0) // -1 indica un slot vacío
			{
				vertex.m_Weights[i] = weight;
				vertex.m_BoneIDs[i] = boneID;
				return; // Termina en cuanto encuentra un slot
			}
		}
	}

	// --- Función para extraer pesos de los huesos ---
	void ExtractBoneWeightForVertices(vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
	{
		// Itera sobre todos los huesos de esta malla
		for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			int boneID = -1;
			string boneName = mesh->mBones[boneIndex]->mName.C_Str();

			// Si el hueso no está en nuestro mapa global, lo añadimos
			if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
			{
				BoneInfo newBoneInfo;
				newBoneInfo.id = m_BoneCounter; // Asigna un nuevo ID
				newBoneInfo.offset = AssimpToGLMmat4(mesh->mBones[boneIndex]->mOffsetMatrix); // Guarda su offset matrix
				m_BoneInfoMap[boneName] = newBoneInfo;
				boneID = m_BoneCounter;
				m_BoneCounter++; // Incrementa el contador global de huesos
			}
			else // Si ya estaba, solo obtenemos su ID
			{
				boneID = m_BoneInfoMap[boneName].id;
			}

			assert(boneID != -1);

			// Ahora, asignamos los pesos de este hueso a los vértices correspondientes
			auto weights = mesh->mBones[boneIndex]->mWeights;
			int numWeights = mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
				assert(vertexId < vertices.size());

				SetVertexBoneData(vertices[vertexId], boneID, weight);
			}
		}
	}

	// Procesa una malla
	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		vector<Vertex> vertices;
		vector<GLuint> indices;
		vector<Texture> textures;

		// 1. Recorrer los vértices de la malla
		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;

			// --- Inicializar datos de hueso ---
			for (int j = 0; j < MAX_BONE_INFLUENCE; j++) {
				vertex.m_BoneIDs[j] = -1;
				vertex.m_Weights[j] = 0.0f;
			}

			// Posición
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			// Normales
			if (mesh->HasNormals())
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}

			// Coordenadas de Textura
			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
			{
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}

			vertices.push_back(vertex);
		}

		// 2. Recorrer las caras (índices)
		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (GLuint j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		// 3. Extraer los datos de hueso para los vértices
		ExtractBoneWeightForVertices(vertices, mesh, scene);

		// 4. Procesar materiales (texturas)
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// 1. Mapas de Difuso
			vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			// 2. Mapas de Especular
			vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		// Devolver la malla completa
		return Mesh(vertices, indices, textures);
	}

	// Carga las texturas de un material
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;

		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);

			// Evitar cargar la misma textura múltiples veces
			GLboolean skip = false;
			for (GLuint j = 0; j < textures_loaded.size(); j++)
			{
				if (textures_loaded[j].path == str)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}

			if (!skip)
			{	// Si no está cargada, cargarla
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str; // Usamos el aiString original
				textures.push_back(texture);

				this->textures_loaded.push_back(texture); // Añadir a la lista de cargadas
			}
		}

		return textures;
	}
};

GLint TextureFromFile(const char* path, string directory)
{
	//Generar ID de textura y cargar datos
	string filename = string(path);
	std::replace(filename.begin(), filename.end(), '\\', '/'); // <-- Normalizar ruta
	filename = directory + '/' + filename;

	std::cout << "========================================" << std::endl;
	std::cout << "INTENTANDO CARGAR TEXTURA:" << std::endl;
	std::cout << "  > Ruta (desde FBX): " << path << std::endl;
	std::cout << "  > Directorio Base (del Modelo): " << directory << std::endl;
	std::cout << "  > RUTA FINAL: " << filename << std::endl;

	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;
	unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);

	std::cout << "  > Dimensiones (w, h): " << width << ", " << height << std::endl;

	if (image == nullptr)
	{
		std::cout << "  > ERROR: ¡NO SE PUDO CARGAR LA IMAGEN!" << std::endl;
		std::cout << "  > Error SOIL: " << SOIL_last_result() << std::endl;

		glDeleteTextures(1, &textureID);
		return 0;
	}
	else
	{
		std::cout << "  > EXITO: Textura cargada." << std::endl;
	}
	std::cout << "========================================" << std::endl;

	// Asignar textura al ID
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Corregir alineación de OpenGL para texturas con ancho no-múltiplo de 4
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0); // Desenlazar

	// Regresar alineación a su default
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	SOIL_free_image_data(image); // Liberar memoria

	return textureID;
}