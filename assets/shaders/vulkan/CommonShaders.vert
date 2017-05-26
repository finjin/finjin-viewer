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


//Output
layout (location=0) out float2 outTextureCoordinate;
out gl_PerVertex 
{
    float4 gl_Position; //homogeneousPosition
};


//Entry point------------------------------------------------------------------
void main() 
{
	//Render a quad without a buffer https://rauwendaal.net/2014/06/14/rendering-a-screen-covering-triangle-in-opengl/
	
#if ALWAYS_FULLSCREEN
    //Use a triangle
	//Due to the fact a triangle is being used, the illusion of a quad is broken if it is scaled or translated
	outTextureCoordinate = float2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = float4(outTextureCoordinate * 2.0f - 1.0f, 0.0f, 1.0f);
#else	
	//Use a triangle strip
	switch (gl_VertexIndex)
	{
		case 0: gl_Position = float4(-1, 1, 0, 1); outTextureCoordinate = float2(0, 0); break; //Upper left		
		case 1: gl_Position = float4(1, 1, 0, 1); outTextureCoordinate = float2(1, 0); break; //Upper right
		case 2: gl_Position = float4(-1, -1, 0, 1); outTextureCoordinate = float2(0, 1); break; //Lower left
		case 3: gl_Position = float4(1, -1, 0, 1); outTextureCoordinate = float2(1, 1); break; //Lower right
		default: gl_Position = float4(0, 0, 0, 0); outTextureCoordinate = float2(0, 0); break;
    }
	gl_Position = modelViewProjectionMatrix * gl_Position;
#endif
}
