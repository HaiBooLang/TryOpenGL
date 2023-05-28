#version 420 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

void main()
{   
    vec3 norm = normalize(Normal);
    if(gl_FragCoord.x < 0)
        FragColor = vec4(norm.x, norm.y, norm.z, 1.0);
    else
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);        
}