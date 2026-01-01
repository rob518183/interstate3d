#version 460 core

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform vec3 sunDirection = vec3(0.3, 0.7, 0.2);
uniform vec3 sunColor = vec3(1.0, 0.9, 0.7);
uniform float timeOfDay = 0.5; // 0 = midnight, 0.5 = noon, 1.0 = midnight

void main() {
    vec3 color = texture(skybox, TexCoords).rgb;
    
    // Simple sun rendering
    float sun = pow(max(dot(TexCoords, sunDirection), 0.0), 512.0);
    color += sun * sunColor * 2.0;
    
    // Atmospheric scattering based on time of day
    vec3 horizonColor = mix(
        vec3(0.1, 0.1, 0.2), // Night
        vec3(0.8, 0.9, 1.0), // Day
        smoothstep(0.2, 0.8, timeOfDay)
    );
    
    float height = TexCoords.y;
    if (height > 0.0) {
        color = mix(horizonColor, color, height);
    } else {
        color = horizonColor;
    }
    
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}
