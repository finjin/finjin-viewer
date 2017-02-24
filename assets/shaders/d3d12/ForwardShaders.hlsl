
//Includes---------------------------------------------------------------------
#include "Utilities.hlsl"


//Macros-----------------------------------------------------------------------
#if !defined(MAX_MATERIALS)
    #define MAX_MATERIALS 200
#endif

#if !defined(MAX_LIGHTS)
    #define MAX_LIGHTS 200
#endif


//Globals----------------------------------------------------------------------
ConstantBuffer<ObjectData> objectData : register(b0, space0);

ConstantBuffer<PassData> passData : register(b1, space0);

cbuffer MaterialCBuffer : register(b2, space0)
{
    MaterialData materials[MAX_MATERIALS];
};

cbuffer LightCBuffer : register(b3, space0)
{
    LightData lights[MAX_LIGHTS];
};

Texture2D textures2D[1000] : register(t0, space0);
TextureCube texturesCube[1000] : register(t1000, space0);
Texture3D textures3D[1000] : register(t2000, space0);

SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);


//Functions--------------------------------------------------------------------
float4 GetCubeFaceColor(float3 position, float3 normal, uint textureIndex)
{
    //Gets the environment map color, given the specified position and normal

    //The direction and normal need to be transformed into environment mapping space so that cube map faces are accessed correctly    
    float3 environmentViewDirection = mul(passData.worldEyePosition - position, (float3x3)passData.environmentMatrix);
    float3 environmentNormal = mul(normal, (float3x3)passData.environmentMatrix);
    if (textureIndex < 1000)
        return textures2D[textureIndex].Sample(anisotropicWrapSampler, reflect(environmentViewDirection, environmentNormal).xy);    
    else
        return texturesCube[textureIndex - 1000].Sample(anisotropicWrapSampler, reflect(environmentViewDirection, environmentNormal));    
}


//Entry points-----------------------------------------------------------------
struct VertexIn
{
    float3 localPosition : POSITION;
    float3 localNormal : NORMAL;
    float4 localTangent : TANGENT;
    float2 localTextureCoordinate0 : TEXCOORD0;
};

struct PixelIn
{
    float4 homogeneousPosition : SV_Position;
    float3 worldPosition : POSITION;
    float3 worldNormal : NORMAL;
    float4 worldTangent : TANGENT;
    float2 textureCoordinate0 : TEXCOORD0;
};

PixelIn VSMain(VertexIn vin)
{
    PixelIn vout;
    
    float4 worldPosition = mul(float4(vin.localPosition, 1.0f), objectData.worldMatrix); //Transform position from local to world space
    
    vout.homogeneousPosition = mul(worldPosition, passData.viewProjectionMatrix); //Transform world position to homogeneous clip space    
    vout.worldPosition = worldPosition.xyz;
    vout.worldNormal = mul(float4(vin.localNormal, 0.0f), objectData.inverseTransposeWorldMatrix).xyz; //Transform normal from local to world space    
    vout.worldTangent = float4(mul(float4(vin.localTangent.xyz, 0.0f), objectData.inverseTransposeWorldMatrix).xyz, vin.localTangent.w); //Transform tangent from local to world space, maintaining parity (w) value
    vout.textureCoordinate0 = vin.localTextureCoordinate0; //Copy texture coordinate
    
    return vout;
}

float4 PSMain(PixelIn pin, bool isFrontFace : SV_IsFrontFace) : SV_Target
{
    //Get some initial data
    MaterialData material = materials[objectData.materialIndex];    
    //float normalScale = BoolToScaledFloat(isFrontFace);
    
    //Normalize interpolated normal/tangent
    pin.worldNormal = normalize(pin.worldNormal);
    
    //Generate a bumped normal if necessary
    #if MAP_BUMP_ENABLED
        float3 worldTangent = normalize(pin.worldTangent.xyz);
        
        //Generate the binormal vector that is perpendicular to the tangent and normal
        float3 worldBinormal = normalize(cross(worldTangent, pin.worldNormal)) * pin.worldTangent.w;
        
        //Get a normal from the bump texture map
        float3 bumpMapNormal = normalize(textures2D[material.bumpTextureIndex].Sample(pointWrapSampler, pin.textureCoordinate0).rgb);
        
        //Move bump map normal to world space and use it as the new normal
        pin.worldNormal = mul(bumpMapNormal, float3x3(worldTangent, worldBinormal, pin.worldNormal));
    #endif
    
    //Get the surface color
    #if MAP_DIFFUSE_ENABLED
        float4 surfaceColor = textures2D[material.diffuseTextureIndex].Sample(anisotropicWrapSampler, pin.textureCoordinate0);
    #else
        float4 surfaceColor = 1.0f;
    #endif
    
    //Blend with reflection texture map
    #if MAP_REFLECTION_ENABLED
        float4 reflectionColor = GetCubeFaceColor(pin.worldPosition, pin.worldNormal, material.reflectionTextureIndex);
        surfaceColor = float4(lerp(surfaceColor.rgb, reflectionColor.rgb, material.reflectionTextureAmount), surfaceColor.a);
    #endif
    
    //Vector from point being lit to eye
    float3 toEye = normalize(passData.worldEyePosition - pin.worldPosition);

    float3 diffuseLightColor = 0;
        
    #if TOTAL_LIGHT_COUNT > 0
        //Unpack light indices    
        uint lightIndices[4] = {objectData.lightIndex.x, objectData.lightIndex.y, objectData.lightIndex.z, objectData.lightIndex.w}; 
    
        uint lightIndex = 0;
    
        uint i;
        #if DIRECTIONAL_LIGHT_COUNT > 0
            for (i = 0; i < DIRECTIONAL_LIGHT_COUNT; ++i)
                ComputeDirectionalLight(diffuseLightColor, lights[lightIndices[lightIndex++]], material, pin.worldPosition, pin.worldNormal, toEye);
        #endif

        #if POINT_LIGHT_COUNT > 0
            for (i = 0; i < POINT_LIGHT_COUNT; ++i)
                ComputePointLight(diffuseLightColor, lights[lightIndices[lightIndex++]], material, pin.worldPosition, pin.worldNormal, toEye);
        #endif

        #if SPOT_LIGHT_COUNT > 0
            for (i = 0; i < SPOT_LIGHT_COUNT; ++i)
                ComputeSpotLight(diffuseLightColor, lights[lightIndices[lightIndex++]], material, pin.worldPosition, pin.worldNormal, toEye);
        #endif
    #endif
    
    //Calculate the diffuse/ambient color using the light and material colors
    float4 diffuseAmbientColor = (material.diffuseColor * float4(diffuseLightColor, 1.0f)) + (material.ambientColor * objectData.ambientLightColor) + material.selfIlluminationColor;
        
    //Calculate the final lit color with its alpha component
    float4 finalColor = float4(diffuseAmbientColor.rgb * surfaceColor.rgb, surfaceColor.a * material.opacity);

    return finalColor;
}
