#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in int aBoneID;
layout (location = 6) in float aWeight;

// transform matrix
layout(std140, binding = 0) uniform Matrices {
	mat4 projection;
    mat4 view;
};

out VS_OUT {
    vec2 texCoords;
    vec3 FragPos;
    vec3 Normal;
} vs_out;

uniform mat4 model;

void main()
{
    vs_out.texCoords = aTexCoords;
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}