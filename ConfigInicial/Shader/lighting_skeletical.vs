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
    mat4 boneTransform = mat4(0.0);
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if(boneIDs[i] >= 0 && boneIDs[i] < MAX_BONES)
        {
            boneTransform += finalBoneMatrices[boneIDs[i]] * weights[i];
        }
    }

    vec4 deformedPos = boneTransform * vec4(position, 1.0);

    gl_Position = projection * view * model * deformedPos;
    FragPos = vec3(model * deformedPos);

    mat3 boneNormalTransform = mat3(transpose(inverse(boneTransform)));
    vec3 deformedNormal = boneNormalTransform * normal;
    Normal = mat3(transpose(inverse(model))) * deformedNormal;

    TexCoords = texCoords;
}
