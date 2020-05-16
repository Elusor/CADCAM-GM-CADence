#include "ShaderStructs.hlsli"
#include "tesselStructs.hlsli"

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

[domain("isoline")]
VSOut main(
	HSOutConst input,
	float2 uv : SV_DomainLocation,
	const OutputPatch<HSOutCP, NUM_CONTROL_POINTS> patch)
{
	matrix MVP = mul(VP, M); // tranposed order
	VSOut o;
	float len = input.EdgeTessFactor[0];
	float u = uv.x; //min(uv.x * (len - 1) / (len - 2), 1.0f);
	float v = uv.y; //min(uv.y * (len - 1) / (len - 2), 1.0f);
	float4 basisU = BernsteinBasis(u);
	float4 basisV = BernsteinBasis(v);
	float3 p = CubicBezierSum(patch, basisU, basisV);
	//o.PosH = mul(float4(p, 1.0f), gWorldViewProj);
	o.pos = mul(MVP, float4(p, 1.0f));
	o.col = float4(1.0f,1.0f,1.0f, 1.0f);
	return o;
}
