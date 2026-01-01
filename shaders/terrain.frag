#version 460 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Tangent;
    vec4 FragPosLightSpace;
    float Height;
} fs_in;

// Terrain textures
uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D sandTexture;
uniform sampler2D snowTexture;
uniform sampler2D shadowMap;

// Lighting
uniform vec3 viewPos;
uniform int numLights;

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};
uniform Light lights[4];

// Terrain parameters
uniform float snowHeight = 50.0;
uniform float rockHeight = 30.0;
uniform float grassHeight = 10.0;
uniform float sandHeight = 0.0;

void main() {
    vec3 color = vec3(0.0);
    vec3 normal = normalize(fs_in.Normal);
    
    // Terrain texturing based on height
    float height = fs_in.Height;
    
    if (height > snowHeight) {
        // Snow
        color = texture(snowTexture, fs_in.TexCoords * 10.0).rgb;
    } else if (height > rockHeight) {
        // Rock with snow blend
        vec3 rock = texture(rockTexture, fs_in.TexCoords * 8.0).rgb;
        vec3 snow = texture(snowTexture, fs_in.TexCoords * 10.0).rgb;
        float blend = (height - rockHeight) / (snowHeight - rockHeight);
        color = mix(rock, snow, blend);
    } else if (height > grassHeight) {
        // Grass with rock blend
        vec3 grass = texture(grassTexture, fs_in.TexCoords * 12.0).rgb;
        vec3 rock = texture(rockTexture, fs_in.TexCoords * 8.0).rgb;
        float blend = (height - grassHeight) / (rockHeight - grassHeight);
        color = mix(grass, rock, blend);
    } else if (height > sandHeight) {
        // Sand with grass blend
        vec3 sand = texture(sandTexture, fs_in.TexCoords * 6.0).rgb;
        vec3 grass = texture(grassTexture, fs_in.TexCoords * 12.0).rgb;
        float blend = (height - sandHeight) / (grassHeight - sandHeight);
        color = mix(sand, grass, blend);
    } else {
        // Sand
        color = texture(sandTexture, fs_in.TexCoords * 6.0).rgb;
    }
    
    // Basic lighting
    vec3 ambient = color * 0.2;
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    
    for(int i = 0; i < numLights; ++i) {
        vec3 lightDir = normalize(lights[i].position - fs_in.FragPos);
        float distance = length(lights[i].position - fs_in.FragPos);
        float attenuation = 1.0 / (distance * distance);
        
        // Diffuse
        float diff = max(dot(normal, lightDir), 0.0);
        diffuse += diff * lights[i].color * lights[i].intensity * attenuation;
        
        // Specular
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        specular += spec * lights[i].color * lights[i].intensity * attenuation * 0.5;
    }
    
    vec3 result = ambient + (diffuse + specular) * color;
    
    // Shadow calculation (simplified)
    vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float shadow = 0.0;
    if (projCoords.x >= 0.0 && projCoords.x <= 1.0 && 
        projCoords.y >= 0.0 && projCoords.y <= 1.0) {
        shadow = texture(shadowMap, projCoords.xy).r;
        shadow = projCoords.z > shadow ? 0.7 : 0.0;
    }
    
    result *= (1.0 - shadow);
    
    // HDR and gamma correction
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}
