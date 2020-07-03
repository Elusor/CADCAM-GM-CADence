struct VSIn
{
    float3 pos : POSITION;
    float3 col : COLOR;
};

struct PSIn
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORDS0;
    float3 col : COLOR;
};

PSIn main(VSIn i)
{
    PSIn o;
    float3 translated = i.pos;
    translated.x -= 0.5f;
    translated.y -= 0.5f;
    float3 scaled = translated * 2.f;    
    o.pos = float4(scaled, 1.0f);
    o.tex = i.pos.xy;
    o.col = i.col;
    return o;
}