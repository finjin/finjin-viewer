#include "MainProgramHeader.glsl"


//Includes---------------------------------------------------------------------
#include "HLSLUtilities.glsl"


//Bindings---------------------------------------------------------------------

//Push constants
layout (push_constant) uniform PushConstantsBlock
{
#if !ALWAYS_FULLSCREEN
	float4x4 modelViewProjectionMatrix;
#endif
	uint4 textureIndex;
	float4 multiplyColor;
	float4 addColor;
};

//Samplers and textures
layout (set=0, binding=0) uniform sampler defaultTextureSampler;
layout (set=0, binding=1) uniform texture2D textures[60];


//Input
layout (location=0) in float2 inTextureCoordinate;

//Output
layout (location=0) out float4 outFragColor;


//Entry point------------------------------------------------------------------
void main() 
{
	outFragColor = texture(sampler2D(textures[textureIndex.x], defaultTextureSampler), inTextureCoordinate) * multiplyColor + addColor;
}
