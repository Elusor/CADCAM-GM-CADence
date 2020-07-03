#include "ShaderStructs.hlsli"

VSOut main(VSIn i)
{
    VSOut o;
    float3 translated = i.pos;
    translated.x -= 0.5f;
    translated.y -= 0.5f;
    float3 scaled = translated * 2.f;    
    o.pos = float4(scaled, 1.0f);
    o.col = float4(i.col, 1.0f);
    o.posL = o.pos;
    o.posW = o.pos;
    return o;
}