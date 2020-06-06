#include "ShaderStructs.hlsli"

float4 camPos : register(b0);
static const float cutoffNear = 150.f;
static const float cutoffFar = 175.f;

float4 main(VSOut i) : SV_TARGET
{
    float3 camDir = camPos.xyz - i.posW;
    float dist = sqrt(dot(camDir, camDir));    
    float alpha = 1.f - (dist - cutoffNear) / (cutoffFar - cutoffNear);
        
    return float4(i.col.xyz, alpha);
}