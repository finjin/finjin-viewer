
//Macros-----------------------------------------------------------------------
#ifndef DIRECTIONAL_LIGHT_COUNT
    #define DIRECTIONAL_LIGHT_COUNT 0
#endif

#ifndef POINT_LIGHT_COUNT
    #define POINT_LIGHT_COUNT 0
#endif

#ifndef SPOT_LIGHT_COUNT
    #define SPOT_LIGHT_COUNT 0
#endif

#define TOTAL_LIGHT_COUNT DIRECTIONAL_LIGHT_COUNT + POINT_LIGHT_COUNT + SPOT_LIGHT_COUNT


//Structs----------------------------------------------------------------------
struct PassData
{
    float4x4 viewMatrix;    
    float4x4 inverseViewMatrix;
    float4x4 inverseTransposeViewMatrix;
    float4x4 projectionMatrix;
    float4x4 inverseProjectionMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 inverseViewProjectionMatrix;
    float4x4 environmentMatrix;
    float3 worldEyePosition;
    float2 renderTargetSize;
    float2 inverseRenderTargetSize;
    float nearZ;
    float farZ;
    float totalTime;
    float deltaTime;
};

struct ObjectData
{
    float4x4 worldMatrix;
    float4x4 inverseWorldMatrix;
    float4x4 inverseTransposeWorldMatrix;
    float4 ambientLightColor;
    uint4 lightIndex;
    uint materialIndex;
};

struct MaterialData
{
    float4 diffuseColor;
    float4 ambientColor;
    float4 specularColor;
    float4 selfIlluminationColor;
    float4x4 textureCoordinateMatrix;
    uint diffuseTextureIndex;
    uint specularTextureIndex;
    uint reflectionTextureIndex;
    uint refractionTextureIndex;
    uint bumpTextureIndex;    
    uint heightTextureIndex;    
    uint selfIlluminationTextureIndex;
    uint opacityTextureIndex;
    uint environmentTextureIndex;
    uint shininessTextureIndex;
    float shininess;
    float opacity;
    float reflectionTextureAmount;
    float environmentTextureAmount;
};

struct LightData
{
    float4 color;
    float3 direction;
    float3 position;    
    float2 range;
    float2 coneRange;
    float2 sinConeRange;    
    float power;
};


//Functions--------------------------------------------------------------------
float BoolToScaledFloat(bool value)
{
    float result = value; //0.0f or 1.0f. See casting rules: https://msdn.microsoft.com/en-us/library/windows/desktop/bb172396(v=vs.85).aspx
    return result * 2.0f - 1.0f; //Remap to [-1, 1]
}

float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    //Schlick gives an approximation to Fresnel reflectance (see pg. 233 "Real-Time Rendering 3rd Ed.").
    //R0 = ( (n-1)/(n+1) )^2, where n is the index of refraction.

    float cosIncidentAngle = saturate(dot(normal, lightVec));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0)*(f0*f0*f0*f0*f0);

    return reflectPercent;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, MaterialData mat)
{
    const float m = mat.shininess * 100.0f;
    float3 halfVec = normalize(toEye + lightVec);

    float roughnessFactor = (m + 8.0f)*pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    float3 fresnelR0 = float3(0.01f, 0.01f, 0.01f);
    float3 fresnelFactor = SchlickFresnel(fresnelR0, halfVec, lightVec);

    float3 specAlbedo = fresnelFactor*roughnessFactor;

    //Our spec formula goes outside [0,1] range, but we are doing LDR rendering.  Scale it down a bit.
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.diffuseColor.rgb + specAlbedo) * lightStrength;
}

float GetRangePercentage(float value, float innerRange, float outerRange)
{
    //Calculates a value in [0, 1] as follows:
    //if (value < innerRange) 
    // return 1
    //else if (value > outerRange) 
    // return 0
    //else 
    // return interpolated value between 0 and 1
    //This function is used in determining how much influence a ranged light (either omni or spot) has on a vertex
    
    float range = outerRange - innerRange;

    //Use a smooth step to interpolate between 0 and 1
    //It's a minor visual improvement over the linear interpolation
    return smoothstep(0, 1, 1.0f - saturate((value - innerRange) / range));
    
    //Here's the linear version
    //return 1.0f - saturate((value - innerRange) / range);    
}

void ComputeDirectionalLight(inout float3 diffuseLightColor, LightData L, MaterialData mat, float3 pos, float3 normal, float3 toEye)
{
    float3 toLight = L.position - pos;    
    if (dot(toLight, L.direction) > 0)
    {
        //Point is in front of the light
        float distance = length(toLight);
        if (distance < L.range.y)
        {        
            float ndotl = max(dot(L.direction, normal), 0.0f);
            float3 lightStrength = L.power * ndotl * GetRangePercentage(distance, L.range.x, L.range.y);
            
            diffuseLightColor += BlinnPhong(lightStrength, L.direction, normal, toEye, mat);
        }
    }
}

void ComputePointLight(inout float3 diffuseLightColor, LightData L, MaterialData mat, float3 pos, float3 normal, float3 toEye)
{
    float3 toLight = L.position - pos;    
    float distance = length(toLight);
    if (distance < L.range.y)
    {
        toLight /= distance;

        float ndotl = max(dot(toLight, normal), 0.0f);
        float3 lightStrength = L.power * ndotl * GetRangePercentage(distance, L.range.x, L.range.y);

        diffuseLightColor += BlinnPhong(lightStrength, toLight, normal, toEye, mat);
    }
}

void ComputeSpotLight(inout float3 diffuseLightColor, LightData L, MaterialData mat, float3 pos, float3 normal, float3 toEye)
{
    float3 toLight = L.position - pos;    
    float distance = length(toLight);
    toLight /= distance;
    
    float ndotl = dot(toLight, normal);
            
    //Figure out if the point is within the spotlight's distance-bound cone
    float cosAngle = dot(toLight, L.direction);
    float sinAngle = sqrt(1.0f - cosAngle * cosAngle); //Same as sin(acos(cosAngle)) but faster        
    if (sinAngle <= L.sinConeRange.y && distance <= L.range.y)
    {
        //The point is within the spotlight's cone            
        float coneAttenuation = GetRangePercentage(sinAngle, L.sinConeRange.x, L.sinConeRange.y);
        float rangeAttenuation = GetRangePercentage(distance, L.range.x, L.range.y);
        float lightStrength = L.power * ndotl * coneAttenuation * rangeAttenuation;            
        
        diffuseLightColor += BlinnPhong(lightStrength, toLight, normal, toEye, mat);
    }
}
