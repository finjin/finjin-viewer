#include "MainProgramHeader.glsl"


//Includes---------------------------------------------------------------------
#include "ForwardShadersCommon.glsl"


//Bindings---------------------------------------------------------------------

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

//Input
layout (location=0) in vec3 inLocalPosition;
layout (location=1) in vec3 inLocalNormal;
layout (location=2) in vec4 inLocalTangent;
layout (location=3) in vec2 inLocalTextureCoordinate0;

layout (push_constant) uniform PushConstantsBlock
{
	int objectIndex;
};

//Output
layout (location=0) out vec3 outWorldPosition;
layout (location=1) out vec3 outWorldNormal;
layout (location=2) out vec4 outWorldTangent;
layout (location=3) out vec2 outTextureCoordinate0;
out gl_PerVertex 
{
    vec4 gl_Position; //homogeneousPosition
};


//Entry points-----------------------------------------------------------------
void main() 
{
	ObjectData objectData = objects[objectIndex];
	
	vec4 worldPosition = objectData.worldMatrix * vec4(inLocalPosition.xyz, 1.0f); //Transform position from local to world space
	
	outWorldPosition = worldPosition.xyz;
    outWorldNormal = (objectData.inverseTransposeWorldMatrix * vec4(inLocalNormal, 0.0f)).xyz; //Transform normal from local to world space    
    outWorldTangent = vec4((objectData.inverseTransposeWorldMatrix * vec4(inLocalTangent.xyz, 0.0f)).xyz, inLocalTangent.w); //Transform tangent from local to world space, maintaining parity (w) value
    outTextureCoordinate0 = inLocalTextureCoordinate0; //Copy texture coordinate
	
	gl_Position = passData.viewProjectionMatrix * worldPosition;
}
