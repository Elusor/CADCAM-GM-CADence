#include "ShaderStructs.hlsli"

cbuffer transformations : register(b0)
{
	matrix VP; // view projection matrix
}

cbuffer transformations : register(b1)
{
	matrix M; // model matrix
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

[instance(20)]
[maxvertexcount(10)]
void main(
	line VSOutParams input[2] : SV_POSITION,
	inout LineStream<VSOut> output,
	uint InstanceID : SV_GSInstanceID
	)
{	
	
	matrix MVP = mul(VP, M); // tranposed order
	int currentGSIntances = 20;
	int subSteps = 5;
	
	float step = 1.0f / (float) (currentGSIntances);
	float subStep = step / (float) subSteps;		
	
	float4 begPos = GetTorusPosition(input[0].params);
	float4 endPos = GetTorusPosition(input[1].params);
	
	float instanceBegParams = step * (float) InstanceID;	
	
	for (int i = 0; i < subSteps; i++)
	{
		float substepBeg = instanceBegParams + (float) i * subStep;
		float substepEnd = instanceBegParams + (float) (i + 1) * subStep;		
	
		float4 begPosL = lerp(begPos, endPos, substepBeg);
		float4 endPosL = lerp(begPos, endPos, substepEnd);
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