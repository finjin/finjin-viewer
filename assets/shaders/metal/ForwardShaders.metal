
//Includes---------------------------------------------------------------------
#include "Utilities.metal"


//Entry points-----------------------------------------------------------------
struct VertexInput
{
    float3 localPosition [[attribute(0)]];
    float3 localNormal [[attribute(1)]];
    float4 localTangent [[attribute(2)]];
    float2 localTextureCoordinate0 [[attribute(3)]];
};

struct VertexOutput
{
    float4 homogeneousPosition [[position]];
    float3 worldPosition;
    float3 worldNormal;
    float4 worldTangent;
    float2 textureCoordinate0;
};

struct FragmentOutput
{
    float4 c [[color(0)]];
};

//Vertex shader function
vertex VertexOutput vertex_main
    (
    VertexInput vin [[stage_in]],
    constant PassData& passData [[buffer(1)]],
    constant ObjectData& objectData [[buffer(2)]],
    constant MaterialData& materialData [[buffer(3)]],
    constant LightData& lightData [[buffer(4)]],
    constant LightData& lightData2 [[buffer(5)]],
    constant LightData& lightData3 [[buffer(6)]],
    constant LightData& lightData4 [[buffer(7)]],
    sampler defaultSampler [[sampler(0)]]
#if MAP_DIFFUSE_ENABLED
    ,texture2d<float> diffuseTexture [[texture(0)]]
#endif
    )
{
    VertexOutput vout;
    
    float4 worldPosition = objectData.worldMatrix * float4(vin.localPosition.xyz, 1.0f); //Transform position from local to world space
    
    vout.homogeneousPosition = passData.viewProjectionMatrix * worldPosition;
    vout.worldPosition = worldPosition.xyz;
    vout.worldNormal = (objectData.inverseTransposeWorldMatrix * float4(vin.localNormal, 0.0f)).xyz; //Transform normal from local to world space    
    vout.worldTangent = float4((objectData.inverseTransposeWorldMatrix * float4(vin.localTangent.xyz, 0.0f)).xyz, vin.localTangent.w); //Transform tangent from local to world space, maintaining parity (w) value
    vout.textureCoordinate0 = vin.localTextureCoordinate0; //Copy texture coordinate
    
    return vout;
}

//Fragment shader function
fragment FragmentOutput fragment_main
    (
    VertexOutput fragmentIn [[stage_in]],
    constant PassData& passData [[buffer(1)]],
    constant ObjectData& objectData [[buffer(2)]],
    constant MaterialData& materialData [[buffer(3)]],
    constant LightData& lightData [[buffer(4)]],
    constant LightData& lightData2 [[buffer(5)]],
    constant LightData& lightData3 [[buffer(6)]],
    constant LightData& lightData4 [[buffer(7)]],
    sampler defaultSampler [[sampler(0)]]
#if MAP_DIFFUSE_ENABLED
    ,texture2d<float> diffuseTexture [[texture(0)]]
#endif
    )
{
    FragmentOutput fout;

    //Get the surface color
    #if MAP_DIFFUSE_ENABLED
        float4 surfaceColor = diffuseTexture.sample(defaultSampler, fragmentIn.textureCoordinate0); 
    #else
        float4 surfaceColor = float4(1.0, 1.0, 1.0, 1.0);
    #endif
    
    //Vector from point being lit to eye
    float3 toEye = normalize(passData.worldEyePosition - fragmentIn.worldPosition);

    float3 diffuseLightColor = float3(0, 0, 0);
    
    diffuseLightColor += ComputeDirectionalLight(lightData, materialData, fragmentIn.worldPosition, fragmentIn.worldNormal, toEye);
    
    //Calculate the diffuse/ambient color using the light and material colors
    float4 diffuseAmbientColor = (materialData.diffuseColor * float4(diffuseLightColor, 1.0f)) + (materialData.ambientColor * objectData.ambientLightColor) + materialData.selfIlluminationColor;
        
    //Calculate the final lit color with its alpha component
    fout.c = float4(diffuseAmbientColor.rgb * surfaceColor.rgb, surfaceColor.a * materialData.opacity);
    
    return fout;
}
