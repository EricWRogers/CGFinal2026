[OPENGL VERSION]

in vec3 fragmentNormal;
in vec2 fragmentUV;
in vec3 fragmentWorldPos;

uniform sampler2D albedoMap;
uniform sampler2D specularMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;

uniform bool useAlbedoMap;
uniform bool useSpecularMap;
uniform bool useRoughnessMap;
uniform bool useMetallicMap;

uniform vec4 albedoValue;
uniform float specularValue;
uniform float roughnessValue;
uniform float metallicValue;

uniform vec3 cameraPosition;

uniform bool useDirectionalLight;
uniform vec3 directionalLightDirection;
uniform vec3 directionalLightColor;
uniform float directionalLightIntensity;

uniform bool usePointLight;
uniform vec3 pointLightPosition;
uniform vec3 pointLightColor;
uniform float pointLightIntensity;
uniform float pointLightRange;

out vec4 color;

vec3 SRGBToLinear(vec3 value)
{
    return pow(max(value, vec3(0.0)), vec3(2.2));
}

vec3 LinearToSRGB(vec3 value)
{
    return pow(max(value, vec3(0.0)), vec3(1.0 / 2.2));
}

vec3 ACESFilm(vec3 value)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((value * (a * value + b)) / (value * (c * value + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 n = normalize(fragmentNormal);
    vec3 viewDir = normalize(cameraPosition - fragmentWorldPos);

    vec4 albedoTex = useAlbedoMap ? texture(albedoMap, fragmentUV) : vec4(1.0);
    vec4 albedo = albedoValue * albedoTex;
    vec3 albedoLinear = SRGBToLinear(albedo.rgb);

    float specularTex = useSpecularMap ? texture(specularMap, fragmentUV).r : 1.0;
    float roughnessTex = useRoughnessMap ? texture(roughnessMap, fragmentUV).r : 1.0;
    float metallicTex = useMetallicMap ? texture(metallicMap, fragmentUV).r : 1.0;

    float specular = clamp(specularValue * specularTex, 0.0, 1.0);
    float roughness = clamp(roughnessValue * roughnessTex, 0.0, 1.0);
    float metallic = clamp(metallicValue * metallicTex, 0.0, 1.0);

    vec3 litColor = albedoLinear * 0.04; // ambient

    if (useDirectionalLight)
    {
        vec3 lightDir = normalize(-directionalLightDirection);
        vec3 halfDir = normalize(lightDir + viewDir);
        float diffuse = max(dot(n, lightDir), 0.0);
        float specularTerm = pow(max(dot(n, halfDir), 0.0), 32.0);

        float diffuseLighting = diffuse * (1.0 - 0.4 * roughness);
        vec3 diffuseColor = albedoLinear * diffuseLighting * mix(1.0, 0.65, metallic);
        vec3 specColor = vec3(specular * specularTerm * (1.0 - 0.5 * roughness));
        litColor += (diffuseColor + specColor) * directionalLightColor * directionalLightIntensity;
    }

    if (usePointLight)
    {
        vec3 toLight = pointLightPosition - fragmentWorldPos;
        float distanceToLight = length(toLight);
        vec3 lightDir = (distanceToLight > 0.0001) ? (toLight / distanceToLight) : vec3(0.0, 1.0, 0.0);

        float attenuation = 1.0;
        if (pointLightRange > 0.0001)
        {
            float rangeFactor = clamp(1.0 - (distanceToLight / pointLightRange), 0.0, 1.0);
            attenuation = rangeFactor * rangeFactor;
        }

        vec3 halfDir = normalize(lightDir + viewDir);
        float diffuse = max(dot(n, lightDir), 0.0);
        float specularTerm = pow(max(dot(n, halfDir), 0.0), 32.0);

        float diffuseLighting = diffuse * (1.0 - 0.4 * roughness);
        vec3 diffuseColor = albedoLinear * diffuseLighting * mix(1.0, 0.65, metallic);
        vec3 specColor = vec3(specular * specularTerm * (1.0 - 0.5 * roughness));
        litColor += (diffuseColor + specColor) * pointLightColor * pointLightIntensity * attenuation;
    }

    color = vec4(LinearToSRGB(ACESFilm(litColor)), albedo.a);
}
