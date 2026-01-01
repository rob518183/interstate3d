#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

out vec3 FragPos;
out vec4 VertexColor;

uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = aPos;
    VertexColor = aColor;
    
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = 10.0; // Size of voxel points
}
