#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main() {
    TexCoords = aPos;
    
    // Remove translation from view matrix for skybox
    mat4 skyboxView = mat4(mat3(view));
    
    gl_Position = projection * skyboxView * vec4(aPos, 1.0);
    gl_Position = gl_Position.xyww; // Ensure depth is always 1.0
}
