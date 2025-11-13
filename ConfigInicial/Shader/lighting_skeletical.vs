#version 330 core

#define MAX_BONE_INFLUENCE 4
#define MAX_BONES 200

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

layout (location = 3) in ivec4 boneIDs;
layout (location = 4) in vec4 weights;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 finalBoneMatrices[MAX_BONES];

void main()
{
    // Empezar con la matriz identidad.
    // Si no hay pesos, la transformación es "no hacer nada".
    mat4 boneTransform = mat4(1.0);

    // Solo acumular si el vértice está realmente "skineado"
    if(weights.x > 0.0 || weights.y > 0.0 || weights.z > 0.0 || weights.w > 0.0)
    {
        boneTransform = mat4(0.0); // Resetear a cero para la acumulación
        for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if(boneIDs[i] >= 0 && boneIDs[i] < MAX_BONES)
            {
                boneTransform += finalBoneMatrices[boneIDs[i]] * weights[i];
            }
        }
    }

    vec4 deformedPos = boneTransform * vec4(position, 1.0);
    
    gl_Position = projection * view * model * deformedPos;
    FragPos = vec3(model * deformedPos);

    // La transformación de la normal también debe usar 'boneTransform'
    mat3 boneNormalTransform = mat3(1.0);
    if (determinant(boneTransform) > 0.0001)
    {
        boneNormalTransform = mat3(transpose(inverse(boneTransform)));
    }
    
    vec3 deformedNormal = boneNormalTransform * normal;
    Normal = mat3(transpose(inverse(model))) * deformedNormal;

    TexCoords = texCoords;
}