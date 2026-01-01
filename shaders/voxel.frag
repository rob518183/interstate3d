#version 460 core

in vec3 FragPos;
in vec4 VertexColor;

out vec4 FragColor;

uniform vec3 viewPos;

void main() {
    // Basic voxel rendering with lighting
    vec3 color = VertexColor.rgb;
    float alpha = VertexColor.a;
    
    // Simple distance-based fog
    float distance = length(FragPos - viewPos);
    float fogFactor = exp(-distance * 0.01);
    
    color = mix(vec3(0.5, 0.6, 0.7), color, fogFactor);
    
    FragColor = vec4(color, alpha);
}
