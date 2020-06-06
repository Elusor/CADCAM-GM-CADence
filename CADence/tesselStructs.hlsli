// Output control point
struct HSOutCP
{
    float4 pos : POSITION;
	float3 posL : WORLDPOS;
	float3 color : COLOR;
};

// Output patch constant data.
struct HSOutConst
{
	float EdgeTessFactor[2]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	//float InsideTessFactor[2]		: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};