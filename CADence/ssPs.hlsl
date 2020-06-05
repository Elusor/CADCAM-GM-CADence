// used to draw in screen space

struct PsIn
{
    float4 pos : SV_Position;
    float4 col : COLOR;
};

float4 main(PsIn i) : SV_TARGET
{
	return i.col;
}