#version 330 core

#define MAX_TEXTURE_NUM 8

struct Material {
    sampler2D texture_diffuse[MAX_TEXTURE_NUM];
    sampler2D texture_specular[MAX_TEXTURE_NUM];
    int texture_diffuse_num;
    int texture_specular_num;
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
  // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // =======================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // =======================================================
    // phase 1: directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    // phase 2: point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);

    // phase 3: spot light
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    
    
    vec3 I = normalize(FragPos - viewPos);
    vec3 R = reflect(I, normalize(Normal));
    // result += vec3(texture(skybox, R).rgb);
    //vec3 I = normalize(FragPos - viewPos);
    //vec3 R = reflect(I, normalize(norm));

    //result += vec3(texture(skybox, R).rgb) / 2.0;

    FragColor = vec4(vec3(texture(skybox, R).rgb), 1.0);
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
    specular = lightSpecular * spec * specular;
    return ambient + diffuse + specular;
}
