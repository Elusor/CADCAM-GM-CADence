#include "ShaderStructs.hlsli"
#include "tesselStructs.hlsli"
#include "bezierPatchFuncs.hlsli"

#define NUM_CONTROL_POINTS 16

cbuffer transformations : register(b0) {
	matrix VP; // view projection matrix
}

cbuffer transformations : register(b1) {
	matrix M; // model matrix
}

float4 BernsteinBasis(float t)
{
	float invT = 1.0f - t;

	return float4(invT * invT * invT,
		3.0f * t * invT * invT,
		3.0f * t * t * invT,
		t * t * t);
}

float4 dBernsteinBasis(float t)
{
	float invT = 1.0f - t;

	return float4(-3 * invT * invT,
		3 * invT * invT - 6 * t * invT,
		6 * t * invT - 3 * t * t,
		3 * t * t);
}

float3 CubicBezierSum(const OutputPatch<HSOutCP, 16> bezpatch, float4 basisU, float4 basisV)
{
	float3 sum = float3(0.0f, 0.0f, 0.0f);
	sum = basisV.x * (basisU.x * bezpatch[0].posL + basisU.y * bezpatch[1].posL + basisU.z * bezpatch[2].posL + basisU.w * bezpatch[3].posL);
	sum += basisV.y * (basisU.x * bezpatch[4].posL + basisU.y * bezpatch[5].posL + basisU.z * bezpatch[6].posL + basisU.w * bezpatch[7].posL);
	sum += basisV.z * (basisU.x * bezpatch[8].posL + basisU.y * bezpatch[9].posL + basisU.z * bezpatch[10].posL + basisU.w * bezpatch[11].posL);
	sum += basisV.w * (basisU.x * bezpatch[12].posL + basisU.y * bezpatch[13].posL + basisU.z * bezpatch[14].posL + basisU.w * bezpatch[15].posL);

	return sum;
}

[domain("quad")]
VSOut main(
	in float edgeFactors[4] : SV_TessFactor,
	in float insideFactors[2] : SV_InsideTessFactor,
	HSOutConst input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<HSOutCP, NUM_CONTROL_POINTS> patch)
{
	matrix MVP = mul(VP, M); // tranposed order
	VSOut o;
	float len = input.EdgeTessFactor[0];
    float3 controlpoints[16];
    for (int i = 0; i < 16;i++)
    {
        controlpoints[i] = patch[i].posL;
    }
    
    float u = min(domain.x * (len) / (len - 1), 1.0f);
    float v = min(domain.y * (len) / (len - 1), 1.0f);
	
    float3 p = BezierPatchPoint(controlpoints, domain);    
	float3 p1 = lerp(patch[0].posL, patch[3].posL, domain.x);
	float3 p2 = lerp(patch[12].posL, patch[15].posL, domain.x);

	p = lerp(p1, p2, domain.y);
	p = BezierPatchPoint(controlpoints, domain);    
	o.pos = mul(MVP, float4(p, 1.0f));
    o.posL = p;
    o.posW = mul(M, float4(p, 1.0f));
	int idx = (int)(4.f * domain.y);
	o.col = float4(patch[0].color, 1.0f);
	return o;
}
