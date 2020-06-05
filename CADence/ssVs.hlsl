// used to draw in screen space

struct VSIn
{
    float3 pos : POSITION;
    float3 col : COLOR;
};

struct VsOut
{
    float4 pos : SV_Position;
    float4 col : COLOR;
};

VsOut main( VSIn i)
{
    VsOut o;
    o.pos = float4(i.pos, 1.0f);
    o.col = float4(i.col, 1.0f);
    
	return o;
}