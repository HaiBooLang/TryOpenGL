#version 420 core

#define MAX_TEXTURE_NUM 6

struct Material {
    sampler2D texture_diffuse[MAX_TEXTURE_NUM];
    sampler2D texture_specular[MAX_TEXTURE_NUM];
    sampler2D texture_reflection[MAX_TEXTURE_NUM];
    int texture_diffuse_num;
    int texture_specular_num;
    int texture_reflection_num;
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

#define NR_POINT_LIGHTS 4

uniform vec3 viewPos;
uniform samplerCube skybox;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;


// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalLightSum(vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular, float diff, float spec);

out vec4 FragColor;

void main()
{
    const float offset = 1.0 / 10.0;  

    vec3 offsets[9] = vec3[](
        vec3(-offset,  offset,  0.0f),
        vec3( 0.0f,    offset,  0.0f),
        vec3( offset,  offset,  0.0f),
        vec3(-offset,  0.0f,    0.0f),
        vec3( 0.0f,    0.0f,    0.0f),
        vec3( offset,  0.0f,    0.0f),
        vec3(-offset, -offset,  0.0f),
        vec3( 0.0f,   -offset,  0.0f),
        vec3( offset, -offset,  0.0f)
    );

    float kernel[9] = float[](
        2.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 2.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 2.0 / 16  
    );

    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < material.texture_diffuse_num; i++)
    {
        diffuse += texture(material.texture_diffuse[i], TexCoords).rgb;
    }

    vec3 specular = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < material.texture_specular_num; i++)
    {
        vec3 I = normalize(FragPos - viewPos);
        vec3 R = reflect(I, normalize(Normal));
        float reflect_intensity = texture(material.texture_specular[i], TexCoords).r;
        if(reflect_intensity > 0.5) // Only sample reflections when above a certain treshold
            specular += texture(skybox, R).rgb * texture(material.texture_specular[i], TexCoords).rgb;
    }

    vec3 reflection = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < material.texture_reflection_num; i++)
    {
        vec3 I = normalize(FragPos - viewPos);
        vec3 R = reflect(I, normalize(Normal));

        vec3 skyboxcolor[9];
        for(int i = 0; i < 9; i++)
        {
            skyboxcolor[i] = texture(skybox, R + offsets[i]).rgb;
        }

        vec3 color = vec3(0.0f);
        for(int i = 0; i < 9; i++)
            color += skyboxcolor[i] * kernel[i];

        reflection += color * texture(material.texture_reflection[i], TexCoords).rgb;
    }

    vec3 result = diffuse + specular * 0.6 + reflection * 0.4;

    // Combine them
    FragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    return CalLightSum(light.ambient, light.diffuse, light.specular, diff, spec);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    return CalLightSum(light.ambient, light.diffuse, light.specular, diff, spec) * attenuation;
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    return CalLightSum(light.ambient, light.diffuse, light.specular, diff, spec) * attenuation * intensity;
}

vec3 CalLightSum(vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular, float diff, float spec)
{
    vec3 diffuse = vec3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < material.texture_diffuse_num; i++)
    {
        diffuse += texture(material.texture_diffuse[i], TexCoords).rgb;
    }
    vec3 ambient = lightAmbient * diffuse;
    diffuse = lightDiffuse * diff * diffuse;
    vec3 specular = vec3(0.0,0.0,0.0);
    for(int i = 0; i < material.texture_specular_num; i++)
    {
        specular += texture(material.texture_specular[i], TexCoords).rgb;
    }
    vec3 I = normalize(FragPos - viewPos);
    vec3 R = reflect(I, normalize(Normal));
    specular *= vec3(texture(skybox, R).rgb);
    return ambient + diffuse + specular;
}
