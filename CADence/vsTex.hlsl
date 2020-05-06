struct VSIn
{
	float3 pos : POSITION;
	float3 col : COLOR;
};

struct PSIn
{
	float4 pos : SV_POSITION;
	float2 tex: TEXCOORDS0;
};

PSIn main(VSIn i)
{
	PSIn o;
	o.pos = float4(i.pos,1.0f);
	o.tex = i.pos.xy;
	return o;
}