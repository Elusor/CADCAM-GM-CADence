#include "ShaderStructs.hlsli"

cbuffer transformations : register(b0)
{
    matrix M; // model matrix
}

cbuffer transformations : register(b1)
{
	matrix VP; // view projection matrix
}

cbuffer TorusData : register(b2)
{
	float4 torusData;
}

float4 GetTorusPosition(float2 params)
{
	float u = params.x;
	float v = params.y;
	
	float donutR = torusData.x;
	float tubeR = torusData.y;
	
	float x = (donutR + tubeR * cos(v)) * cos(u);
	float y = tubeR * sin(v);
	float z = (donutR + tubeR * cos(v)) * sin(u);
	
	return float4(x, y, z, 1.f);
}

[instance(1)]
[maxvertexcount(10)]
void main(
	line VSOutParams input[2] : SV_POSITION,
	inout LineStream<VSOut> output,
	uint InstanceID : SV_GSInstanceID
	)
{	
	
    // TO MAKE TORUS MORE SMOOTH INCREASE SUBSTEPS ( <= 5) AND INSTANCES (<= 32)
	matrix MVP = mul(VP, M); // tranposed order
	int currentGSIntances = 1;
	int subSteps = 2;
	
	float step = 1.0f / (float) (currentGSIntances);
	float subStep = step / (float) subSteps;		
	
    float2 paramBeg = input[0].params;
    float2 paramEnd = input[1].params;	
	
	float instanceBegParams = step * (float) InstanceID;	
	
	for (int i = 0; i < subSteps; i++)
	{
		float substepBeg = instanceBegParams + (float) i * subStep;
		float substepEnd = instanceBegParams + (float) (i + 1) * subStep;		
	
        float4 begPosL = GetTorusPosition(lerp(paramBeg, paramEnd, substepBeg));
        float4 endPosL = GetTorusPosition(lerp(paramBeg, paramEnd, substepEnd));
		float4 begCol = lerp(input[0].col, input[1].col, substepBeg);
		float4 endCol = lerp(input[0].col, input[1].col, substepEnd);
					
		VSOut beg;
		beg.pos = mul(MVP, begPosL);
		beg.posW = mul(M, begPosL);
		beg.posL = begPosL;
		beg.col = begCol;
		output.Append(beg);
	
		VSOut end;
		end.pos = mul(MVP, endPosL);
		end.posW = mul(M, endPosL);
		end.posL = endPosL;
		end.col = endCol;
		output.Append(end);
	}
}