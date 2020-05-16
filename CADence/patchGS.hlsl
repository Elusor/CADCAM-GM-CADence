#include "ShaderStructs.hlsli"
cbuffer XC : register(b0) {
	matrix xCoords;
};
matrix yCoords: register(b1);
matrix zCoords: register(b2);
matrix VP: register(b3);
// model matrix (and perhaprs other buffer as well) is somehow bound incorrectly - resulting in all out vertices to be (0,0,0,0)
cbuffer model: register(b4) {
	matrix M;
}

// Change basis matrix - bernstein to power (for coordinates in basis) / power to bernstein (for basis' vectors)
static const float4x4 PToBB = float4x4(
	 1,  0,  0, 0,
	-3,  3,  0, 0,
	 3, -6,  3, 0,
	 1,  3, -3, 1);

float CalculateCoordinate(float4 uVec, float4 vVec, float4x4 A)
{
	// A is the matrix of coordinates - that is why we use the inversion of change basis matrix 
	// u * BtoP * A * BtoP^T * v
	float4 res = mul(uVec, PToBB);
	res = mul(res, A);
	res = mul(res, transpose(PToBB));
	float coord = mul(res, vVec);
	return coord;
}

float3 CalculateCoords(float u, float v)
{
	float4 uVec = float4(1, u, u * u, u * u * u);
	float4 vVec = float4(1, v, v * v, v * v * v);

	float x = CalculateCoordinate(uVec, vVec, xCoords);
	float y = CalculateCoordinate(uVec, vVec, yCoords);
	float z = CalculateCoordinate(uVec, vVec, zCoords);
	return float3(x, y, z);
}

[instance(20)]
[maxvertexcount(64)]
void main(
	point VSOut input[1] : SV_POSITION,
	uint InstanceID : SV_GSInstanceID,
	inout LineStream<VSOut> output){
	matrix MVP = mul(VP, M);
	float u,v;

	float constVal;
	bool uUsed =(input[0].col.y) < 0.f;
	//if u is Used 
	if (uUsed) {
		u = input[0].col.x;
	}
	else {
		v = input[0].col.y;
	}
	int subdivisions = 10;
	float instanceStep = 1.f / 20.f;
	float startParam = InstanceID * instanceStep;
	
	VSOut element;
	element.pos = input[0].pos;
	element.posL = element.pos.xyz;

	element.col = float4(0.8f, 0.0f, 0.0f, 1.0f);
	output.Append(element);

	for (int i = 0; i < subdivisions; i++); // <=?
	{
		float param = startParam + instanceStep * (float)i / float(subdivisions);
		if (uUsed)
		{
			v = param;
		}
		else {
			u = param;
		}

		float3 coords = CalculateCoords(u, v);

		VSOut element;
		element.pos = mul(MVP,float4(coords,1.0f));
		element.posL = element.pos.xyz;
		element.col = float4(0.8f, 0.0f, 0.0f, 1.0f);
		output.Append(element);
	}
	//output.RestartStrip();
}