#include "ShaderStructs.hlsli"
#include "tesselStructs.hlsli"
#include "bezierPatchFuncs.hlsli"

#define NUM_CONTROL_POINTS 20

cbuffer transformations : register(b0)
{
    matrix VP; // view projection matrix
}

cbuffer transformations : register(b1)
{
    matrix M; // model matrix
}


float3 Q11(float3 uPt, float3 vPt, float2 uv)
{
    float u = uv.x;
    float v = uv.y;
    float3 numer = u * uPt + v * vPt;
    float denom = u + v;
    return denom == 0 ? 0 : numer / denom;
}

float3 Q12(float3 uPt, float3 vPt, float2 uv)
{
    float u = uv.x;
    float v = uv.y;
    float3 numer = u * uPt + (1.f - v) * vPt;
    float denom = 1 - v + u;
    return denom == 0 ? 0 : numer / denom;
}

float3 Q21(float3 uPt, float3 vPt, float2 uv)
{
    float u = uv.x;
    float v = uv.y;
    float3 numer = (1.f - u) * uPt + v * vPt;
    float denom = 1 - u + v;
    return denom == 0 ? 0 : numer / denom;
}

float3 Q22(float3 uPt, float3 vPt, float2 uv)
{
    float u = uv.x;
    float v = uv.y;
    float3 numer = (1.f - u) * uPt + (1.f - v) * vPt;
    float denom = 2 - u - v;
    return denom == 0 ? 0 : numer / denom;
}

float3 GregPatchPoint(float3 gregPoints[20], float2 uv)
{
    
    float3 controlpoints[16];
    // Calculate the points that are in the middle of the Greg Patch 
    // They are functions of the two control points and uv params
    
    // 0 - 3 are the same
    for (int i = 0; i < 4; i++)
    {
        controlpoints[i] = gregPoints[i];
    }
    
    // TODO MAYBE SWAP UV HERE? DONT KNOW REALLY
    // second row
    controlpoints[4] = gregPoints[4];
    controlpoints[5] = Q11(gregPoints[5], gregPoints[6], uv);
    controlpoints[6] = Q12(gregPoints[8], gregPoints[7], uv);
    controlpoints[7] = gregPoints[9];
        
    //third row
    controlpoints[8] = gregPoints[10];
    controlpoints[9] = Q21(gregPoints[11], gregPoints[12], uv);
    controlpoints[10] = Q22(gregPoints[14], gregPoints[13], uv);
    controlpoints[11] = gregPoints[15];    
    
    // 16 - 19 are 12 - 15
    for (int j = 12; j < 16; j++)
    {
        controlpoints[j] = gregPoints[j+4];
    }
    
    return BezierPatchPoint(controlpoints, uv);
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
    float3 gregpoints[20];
    for (int i = 0; i < 20; i++)
    {
        gregpoints[i] = patch[i].posL;
    }
    
    float u = min(domain.x * (len) / (len - 1), 1.0f);
    float v = min(domain.y * (len) / (len - 1), 1.0f);
    float2 uv = float2(u, v);
	
    float3 p = GregPatchPoint(gregpoints, uv);
    o.pos = mul(MVP, float4(p, 1.0f));
    o.posL = p;
    o.posW = mul(M, float4(p, 1.0f));
    o.col = float4(patch[0].color, 1.0f);
    return o;
}
