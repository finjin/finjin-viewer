#include "MainProgramHeader.glsl"


//Includes---------------------------------------------------------------------
#include "ForwardShadersCommon.glsl"


//Bindings---------------------------------------------------------------------

//Push constants
layout (push_constant) uniform PushConstantsBlock
{
	int objectIndex;
};

//Uniform buffers
layout (std140, set=0, binding=0) uniform PassDataBlock 
{
	PassData passData;
};
layout (std140, set=0, binding=1) uniform ObjectDataBlock
{
	ObjectData objects[MAX_OBJECTS];
};
layout (std140, set=0, binding=2) uniform LightDataBlock 
{
	LightData lights[MAX_LIGHTS];
};
layout (std140, set=0, binding=3) uniform MaterialDataBlock
{
	MaterialData materials[MAX_MATERIALS];
};

//Samplers and textures
layout (set=1, binding=0) uniform sampler defaultTextureSampler;
layout (set=1, binding=1) uniform texture2D textures[MAX_TEXTURES];


//Input
layout (location=0) in float3 inWorldPosition;
layout (location=1) in float3 inWorldNormal;
layout (location=2) in float4 inWorldTangent;
layout (location=3) in float2 inTextureCoordinate0;

//Output
layout (location=0) out float4 outFragColor;


//Entry point------------------------------------------------------------------
void main() 
{
	//Get some initial data
	ObjectData objectData = objects[objectIndex];
    MaterialData material = materials[objectData.materialIndex];    
    //float normalScale = BoolToScaledFloat(isFrontFace);
    
    //Normalize interpolated normal/tangent
    float3 worldNormal = normalize(inWorldNormal);
    
    //Generate a bumped normal if necessary
    #if MAP_BUMP_ENABLED
        float3 worldTangent = normalize(inWorldTangent.xyz);
        
        //Generate the binormal vector that is perpendicular to the tangent and normal
        float3 worldBinormal = normalize(cross(worldTangent, worldNormal)) * inWorldTangent.w;
        
        //Get a normal from the bump texture map
        float3 bumpMapNormal = normalize(texture(sampler2D(textures[material.bumpTextureIndex], defaultTextureSampler), inTextureCoordinate0).rgb);
        
        //Move bump map normal to world space and use it as the new normal
        worldNormal = float3x3(worldTangent, worldBinormal, worldNormal) * bumpMapNormal;
    #endif
    
    //Get the surface color
    #if MAP_DIFFUSE_ENABLED
        float4 surfaceColor = texture(sampler2D(textures[material.diffuseTextureIndex], defaultTextureSampler), inTextureCoordinate0);
    #else
        float4 surfaceColor = float4(1.0, 1.0, 1.0, 1.0);
    #endif
    
    //Blend with reflection texture map
    #if MAP_REFLECTION_ENABLED
        float4 reflectionColor = GetCubeFaceColor(inWorldPosition, worldNormal, material.reflectionTextureIndex);
        surfaceColor = float4(lerp(surfaceColor.rgb, reflectionColor.rgb, material.reflectionTextureAmount), surfaceColor.a);
    #endif
    
    //Vector from point being lit to eye
    float3 toEye = normalize(passData.worldEyePosition - inWorldPosition);

    float3 diffuseLightColor = float3(0, 0, 0);
        
    #if TOTAL_LIGHT_COUNT > 0
        //Unpack light indices    
        uint lightIndices[4] = {objectData.lightIndex.x, objectData.lightIndex.y, objectData.lightIndex.z, objectData.lightIndex.w}; 
    
        uint lightIndex = 0;
    
        uint i;
        #if DIRECTIONAL_LIGHT_COUNT > 0
            for (i = 0; i < DIRECTIONAL_LIGHT_COUNT; ++i)
                ComputeDirectionalLight(diffuseLightColor, lights[lightIndices[lightIndex++]], material, inWorldPosition, worldNormal, toEye);
        #endif

        #if POINT_LIGHT_COUNT > 0
            for (i = 0; i < POINT_LIGHT_COUNT; ++i)
                ComputePointLight(diffuseLightColor, lights[lightIndices[lightIndex++]], material, inWorldPosition, worldNormal, toEye);
        #endif

        #if SPOT_LIGHT_COUNT > 0
            for (i = 0; i < SPOT_LIGHT_COUNT; ++i)
                ComputeSpotLight(diffuseLightColor, lights[lightIndices[lightIndex++]], material, inWorldPosition, worldNormal, toEye);
        #endif
    #endif
    
    //Calculate the diffuse/ambient color using the light and material colors
    float4 diffuseAmbientColor = (material.diffuseColor * float4(diffuseLightColor, 1.0f)) + (material.ambientColor * objectData.ambientLightColor) + material.selfIlluminationColor;
        
    //Calculate the final lit color with its alpha component
    outFragColor = float4(diffuseAmbientColor.rgb * surfaceColor.rgb, surfaceColor.a * material.opacity);
}
