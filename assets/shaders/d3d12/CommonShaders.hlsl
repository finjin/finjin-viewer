
//Includes---------------------------------------------------------------------
#include "Utilities.hlsl"


//Bindings---------------------------------------------------------------------
#if !ALWAYS_FULLSCREEN
	float4x4 modelViewProjectionMatrix : register(b0);
#endif
uint4 textureIndex : register(b1);
float4 multiplyColor : register(b2);
float4 addColor : register(b3);

Texture2D textures[1000] : register(t0);

SamplerState theSampler : register(s0);


//Entry points-----------------------------------------------------------------
struct VertexOutput
{
    float4 homogeneousPosition : SV_Position;
    float2 textureCoordinate : TEXCOORD0;
};

#if COMPILING_VS
VertexOutput VSMain(uint id : SV_VertexID)
{
	VertexOutput vout;
	
	//Render a quad without a buffer https://rauwendaal.net/2014/06/14/rendering-a-screen-covering-triangle-in-opengl/
	
#if ALWAYS_FULLSCREEN
    //Use a triangle
	//Due to the fact a triangle is being used, the illusion of a quad is broken if it is scaled or translated
	float x = -1.0 + float((id & 1) << 2);
    float y = -1.0 + float((id & 2) << 1);
	vout.homogeneousPosition = float4(x, y, 0, 1);
    vout.textureCoordinate = float2((x + 1.0f) * 0.5f, (y + 1.0f) * 0.5f);
#else	
	//Use a triangle strip
	switch (id)
	{
		case 0: vout.homogeneousPosition = float4(-1, 1, 0, 1); vout.textureCoordinate = float2(0, 0); break; //Upper left		
		case 1: vout.homogeneousPosition = float4(1, 1, 0, 1); vout.textureCoordinate = float2(1, 0); break; //Upper right
		case 2: vout.homogeneousPosition = float4(-1, -1, 0, 1); vout.textureCoordinate = float2(0, 1); break; //Lower left
		case 3: vout.homogeneousPosition = float4(1, -1, 0, 1); vout.textureCoordinate = float2(1, 1); break; //Lower right
		default: vout.homogeneousPosition = float4(0, 0, 0, 0); vout.textureCoordinate = float2(0, 0); break;
    }
	vout.homogeneousPosition = mul(vout.homogeneousPosition, modelViewProjectionMatrix);
#endif
	
	return vout;
}
#endif

#if COMPILING_PS
float4 PSMain(VertexOutput pin) : SV_Target
{	
	return textures[textureIndex.x].Sample(theSampler, pin.textureCoordinate) * multiplyColor + addColor;
}
#endif
