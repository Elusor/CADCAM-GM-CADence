#include "ShaderStructs.hlsli"

cbuffer transformations : register(b0) {
	matrix VP; // view projection matrix
}

cbuffer transformations : register(b1) {
	matrix M; // model matrix
}

VSOut main(VSIn i)
{
	matrix MVP = mul(VP,M); // tranposed order
	VSOut o;
	o.pos = mul(MVP, float4(i.pos, 1.0f));
	o.col = float4(i.col, 1.0f);
	return o;
}