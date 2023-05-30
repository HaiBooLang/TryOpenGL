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

in vec2 TexCoords; 
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform samplerCube skybox;
uniform Material material;

out vec4 FragColor;

void main()
{
    const float offset = 1.0 / 10.0;  

    vec3 offsets[9] = vec3[](
        vec3(-offset,  offset,  0.0f),
        vec3( 0.0f,    offset,  0.0f),
        vec3( offset,  offset,  0.0f),
        vec3(-offset,  0.0f,    0.0f),
        vec3( 0.0f,    0.0f,    offset),
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

    vec3 I = normalize(FragPos - viewPos);
    vec3 R = reflect(I, normalize(Normal));

    vec3 specular = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < material.texture_specular_num; i++)
    {
        vec3 skyboxcolor[9];
        for(int i = 0; i < 9; i++)
        {
            skyboxcolor[i] = texture(skybox, R + offsets[i]).rgb;
        }

        vec3 color = vec3(0.0f);
        for(int i = 0; i < 9; i++)
            color += skyboxcolor[i] * kernel[i];

        specular += color * texture(material.texture_specular[i], TexCoords).rgb;
    }

    vec3 reflection = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < material.texture_reflection_num; i++)
    {
        float reflect_intensity = texture(material.texture_reflection[i], TexCoords).r;
        if(reflect_intensity > 0.1) // Only sample reflections when above a certain treshold
            reflection += texture(skybox, R).rgb * texture(material.texture_reflection[i], TexCoords).rgb;
    }

    vec3 result = diffuse + specular * 0.4 + reflection * 0.6;
    
    FragColor = vec4(result, 1.0f);
}