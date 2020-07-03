#include "ShaderStructs.hlsli"

float4 main(VSOut i) : SV_TARGET
{ 
    return float4(i.col.xyz, 1.0f);
}
