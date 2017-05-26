
//Includes---------------------------------------------------------------------
#include "Utilities.metal"


//Entry points-----------------------------------------------------------------
struct VertexInput
{
    float3 position [[attribute(0)]];
    float2 textureCoordinate [[attribute(1)]];
};

struct FullScreenQuadVertexOutput
{
    float4 homogeneousPosition [[position]];
    float2 textureCoordinate;
};

struct FullScreenQuadFragmentOutput
{
    float4 c [[color(0)]];
};

//Vertex shader function
vertex FullScreenQuadVertexOutput fullscreen_quad_vertex_main
    (
    VertexInput vin [[stage_in]],
    constant float4x4& modelViewProjection [[buffer(1)]]
    )
{
    FullScreenQuadVertexOutput vout;
    
    vout.homogeneousPosition = modelViewProjection * float4(vin.position, 1.0);
    vout.textureCoordinate = vin.textureCoordinate;
    
    return vout;
}

//Fragment shader function
fragment FullScreenQuadFragmentOutput fullscreen_quad_fragment_main
    (
    FullScreenQuadVertexOutput fragmentIn [[stage_in]],
    texture2d<float> fullScreenTexture [[texture(0)]]
    )
{
    constexpr sampler fullScreenQuadSampler(filter::linear);
    
    FullScreenQuadFragmentOutput fout;
    
    fout.c = fullScreenTexture.sample(fullScreenQuadSampler, fragmentIn.textureCoordinate);
    
    return fout;
}
