
//Macros-----------------------------------------------------------------------
#define float4x4 mat4

#define float4 vec4
#define float3 vec3
#define float2 vec2

#define uint4 uvec4
#define uint3 uvec3
#define uint2 uvec2

#define int4 ivec4
#define int3 ivec3
#define int2 ivec2


//Functions--------------------------------------------------------------------
float saturate(float value)
{
    return clamp(value, 0.0f, 1.0);
}
