#version 330 core

#define NUMBER_OF_POINT_LIGHTS 2

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform DirLight  dirLight;
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform int transparency;

// Prototipos
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir );
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir );
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir );

void main()
{
    vec3 norm    = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Tomamos el alfa del difuso (para transparencia), pero NO remezclamos el color aquí
    vec4 albedo = texture(material.diffuse, TexCoords);

    // Iluminación
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; ++i)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    // Transparencia por alfa del difuso
    if (transparency == 1 && albedo.a < 0.1)
        discard;

    color = vec4(result, albedo.a);
}

// Direccional
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir )
{
    vec3 lightDir = normalize(-light.direction);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular (Blinn-Phong opcional; aquí Phong clásico)
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 albedo  = vec3(texture(material.diffuse, TexCoords));
    vec3 specmap = vec3(texture(material.specular, TexCoords));

    vec3 ambient  = light.ambient  * albedo;
    vec3 diffuse  = light.diffuse  * diff * albedo;
    vec3 specular = light.specular * spec * specmap;

    return ambient + diffuse + specular;
}

// Puntual
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 albedo  = vec3(texture(material.diffuse, TexCoords));
    vec3 specmap = vec3(texture(material.specular, TexCoords));

    vec3 ambient  = light.ambient  * albedo;
    vec3 diffuse  = light.diffuse  * diff * albedo;
    vec3 specular = light.specular * spec * specmap;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

// Spot
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Intensidad del cono
    float theta    = dot(lightDir, normalize(-light.direction));
    float epsilon  = max(light.cutOff - light.outerCutOff, 0.0001);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 albedo  = vec3(texture(material.diffuse, TexCoords));
    vec3 specmap = vec3(texture(material.specular, TexCoords));

    vec3 ambient  = light.ambient  * albedo;                 // <- ambient SIN intensity
    vec3 diffuse  = light.diffuse  * diff * albedo;
    vec3 specular = light.specular * spec * specmap;

    ambient  *= attenuation;                                  // sólo atenuación
    diffuse  *= attenuation * intensity;                      // atenuación + cono
    specular *= attenuation * intensity;

    return ambient + diffuse + specular;
}
