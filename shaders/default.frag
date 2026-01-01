#version 460 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Tangent;
    vec3 Bitangent;
    vec4 FragPosLightSpace;
} fs_in;

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform Light lights[4];
uniform int numLights;
uniform vec3 viewPos;

// Material properties
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_metallic1;
uniform sampler2D shadowMap;

uniform vec3 materialColor = vec3(0.8, 0.8, 0.8);
uniform float metallic = 0.0;
uniform float roughness = 0.5;
uniform float ao = 1.0;

const float PI = 3.14159265359;

// Shadow calculation
float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if (projCoords.z > 1.0) {
        return 0.0;
    }
    
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    
    return shadow;
}

// PBR lighting calculation
vec3 calculatePBR(vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness) {
    vec3 Lo = vec3(0.0);
    
    for (int i = 0; i < numLights && i < 4; ++i) {
        vec3 L = normalize(lights[i].position - fs_in.FragPos);
        vec3 H = normalize(V + L);
        
        float distance = length(lights[i].position - fs_in.FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lights[i].color * lights[i].intensity * attenuation;
        
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;
        
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        
        vec3 BRDF = kD * albedo / PI + specular;
        
        float NdotL = max(dot(N, L), 0.0);
        Lo += BRDF * radiance * NdotL;
    }
    
    return Lo;
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // Sample textures
    vec3 albedo = texture(texture_diffuse1, fs_in.TexCoords).rgb * materialColor;
    vec3 normal = texture(texture_normal1, fs_in.TexCoords).rgb;
    float roughnessTex = texture(texture_roughness1, fs_in.TexCoords).r;
    float metallicTex = texture(texture_metallic1, fs_in.TexCoords).r;
    
    // Convert normal from [0,1] to [-1,1]
    normal = normalize(normal * 2.0 - 1.0);
    
    // Create TBN matrix for normal mapping
    vec3 T = normalize(fs_in.Tangent);
    vec3 B = normalize(fs_in.Bitangent);
    vec3 N = normalize(fs_in.Normal);
    mat3 TBN = mat3(T, B, N);
    N = normalize(TBN * normal);
    
    // View direction
    vec3 V = normalize(viewPos - fs_in.FragPos);
    
    // Material properties
    float finalRoughness = roughness * roughnessTex;
    float finalMetallic = metallic + metallicTex;
    vec3 F0 = mix(vec3(0.04), albedo, finalMetallic);
    
    // PBR lighting
    vec3 Lo = calculatePBR(N, V, F0, albedo, finalMetallic, finalRoughness);
    
    // Ambient lighting
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    // Shadow calculation
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    
    vec3 color = ambient + Lo * (1.0 - shadow);
    
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}
