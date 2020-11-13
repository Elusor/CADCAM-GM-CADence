#include "ShaderStructs.hlsli"

cbuffer cambuf : register(b0)
{
    float4 camPos;
}
// near far nearcutoff mode
cbuffer fogbuf : register(b1)
{
    float4 fogBuffer;
};

float LinearizeDepth(float depth)
{
    float near = fogBuffer.x;
    float far = fogBuffer.y;
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

float4 main(VSOut i) : SV_TARGET
{
    float3 col = i.col.xyz;
    
    float near = fogBuffer.x;
    float far = fogBuffer.y;
    float nearCutoff = fogBuffer.z;
    float farCutoff = far;
    bool alternativeFog = fogBuffer.w > 0.0f;
    
    float dist;
    
    if(alternativeFog)
    {
        
        dist = LinearizeDepth(i.pos.z);

    }
    else
    {
        float3 camDir = camPos.xyz - i.posW;
        dist = sqrt(dot(camDir, camDir));       
    }
    
    float alpha = 1 - (dist - nearCutoff) / (farCutoff - nearCutoff);
        
    return float4(col, 1.0f);
}