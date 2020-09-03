struct VSIn
{
	float3 pos : POSITION;
	float3 col : COLOR;
};

struct VSOut
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float3 posL : POSITION0;
	float3 posW : POSITION1;
};

struct VSInParams
{
	float2 params : POSITION;
	float3 col : COLOR;
};

struct VSOutParams
{
	float4 pos : SV_POSITION;
	float2 params : TEXCOORD0;
	float4 col : COLOR;
};

