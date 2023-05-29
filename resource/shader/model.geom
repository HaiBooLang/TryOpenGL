#version 420 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
    vec3 FragPos;
    vec3 Normal;
} gs_in[];

out vec2 TexCoords; 
out vec3 Normal;
out vec3 FragPos;

uniform float time;

vec4 explode(vec4 position, vec3 normal);
vec3 GetNormal();

void main() {    
    vec3 normal = GetNormal();

    gl_Position = explode(gl_in[0].gl_Position, normal);
    TexCoords = gs_in[0].texCoords;
    FragPos = gl_Position.xyz;
    Normal = gs_in[0].Normal;
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal);
    TexCoords = gs_in[1].texCoords;    
    FragPos = gl_Position.xyz;
    Normal = gs_in[1].Normal;
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
    TexCoords = gs_in[2].texCoords;
    FragPos = gl_Position.xyz;
    Normal = gs_in[2].Normal;
    EmitVertex();
    EndPrimitive();
}

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 0.5;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
    return position + vec4(direction, 0.0);
}