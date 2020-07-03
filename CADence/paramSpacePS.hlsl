struct PSIn
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORDS0;
    float3 col : COLOR;

};

float4 main(PSIn i) : SV_TARGET
{ 
    return float4(i.col, 1.0f);
}
