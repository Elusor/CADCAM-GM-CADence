#include "ShaderStructs.hlsli"
#include "tesselStructs.hlsli"
#include "bezierPatchFuncs.hlsli"
#define NUM_CONTROL_POINTS 16

cbuffer transformations : register(b0)
{
    matrix VP; // view projection matrix
}

cbuffer transformations : register(b1)
{
    matrix M; // model matrix
}

[domain("isoline")]
VSOut main(
	HSOutConst input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<HSOutCP, NUM_CONTROL_POINTS> patch)
{
    matrix MVP = mul(VP, M); // tranposed order
    VSOut o;
    float len = input.EdgeTessFactor[0];
    
    float u = min(domain.x * (len) / (len - 1), 1.0f);
    float v = min(domain.y * (len) / (len - 1), 1.0f);
    float2 uv = float2(u, v);
    
    float3 controlPoints[16];
    for (int i = 0; i < 16; i++)
    {
        controlPoints[i] = patch[i].posL;
    }        
    float3 p = BezierC2PatchPoint(controlPoints, uv);
    
    o.pos = mul(MVP, float4(p, 1.0f));
    o.posL = p;
    o.posW = mul(M, float4(p, 1.0f));
    o.col = float4(patch[0].color, 1.0f);
    return o;
}
