#version 420 core

in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform samplerCube skybox;

out vec4 FragColor;

void main()
{
    vec3 I = normalize(FragPos - viewPos);
    vec3 R = reflect(I, normalize(Normal));

    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}