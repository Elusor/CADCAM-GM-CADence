#include "ShaderStructs.hlsli"
#include "bezierPatchFuncs.hlsli"

cbuffer transformations : register(b0)
{
    matrix M; // model matrix
}

cbuffer transformations : register(b1)
{
    matrix VP; // view projection matrix
}

cbuffer patchData1 : register(b2)
{
    float4x4 firstRow;
}

cbuffer patchData2 : register(b3)
{
    float4x4 secondRow;
}

cbuffer patchData3 : register(b4)
{
    float4x4 thirdRow;
}

cbuffer patchData4 : register(b5)
{
    float4x4 fourthRow;
}

// u - row param [0 = column0, 1 = column 3]
// v - column param [0 = row0, 1 = row 3]
//  12 13 14 15
// ^ 8  9 10 11
// | 4  5  6  7
// v 0  1  2  3
// X u-->

[maxvertexcount(2)]
void main(
	line VSOutParams input[2],
	inout LineStream<VSOut> output)
{
    float3 points[16];
    points[0] = firstRow[0].xyz;
    points[1] = firstRow[1].xyz;
    points[2] = firstRow[2].xyz;
    points[3] = firstRow[3].xyz;
    
    points[4] = secondRow[0].xyz;
    points[5] = secondRow[1].xyz;
    points[6] = secondRow[2].xyz;
    points[7] = secondRow[3].xyz;
   
    points[8] = thirdRow[0].xyz;
    points[9] = thirdRow[1].xyz;
    points[10] = thirdRow[2].xyz;
    points[11] = thirdRow[3].xyz;
    
    points[12] = fourthRow[0].xyz;
    points[13] = fourthRow[1].xyz;
    points[14] = fourthRow[2].xyz;
    points[15] = fourthRow[3].xyz;
    
    matrix MVP = mul(VP, M); // tranposed order
    int currentGSIntances = 1;
    int subSteps = 1;

    float step = 1.0f / (float) (currentGSIntances);
    float subStep = step / (float) subSteps;
	
    VSOutParams params1 = input[0];
    VSOutParams params2 = input[1];
    
    float instanceBegParams = step * (float) 0.0f;

    float2 uv1 = input[0].params;
    float2 uv2 = input[1].params;
    
    float4 col1 = input[0].col;
    float4 col2 = input[1].col;
    
    for (int i = 0; i < subSteps; i++)
    {
        float substepBeg = instanceBegParams + (float) i * subStep;
        float substepEnd = instanceBegParams + (float) (i + 1) * subStep;
	
        float2 beg = lerp(uv1, uv2, substepBeg);
        float2 end = lerp(uv1, uv2, substepEnd);
        float4 begPosL = float4(BezierC2PatchPoint(points, beg), 1.0f);
        float4 endPosL = float4(BezierC2PatchPoint(points, end), 1.0f);
        float4 begCol = lerp(col1, col2, substepBeg);
        float4 endCol = lerp(col1, col2, substepEnd);
					
        VSOut begOut;
        begOut.pos = mul(MVP, begPosL);
        begOut.posW = mul(M, begPosL);
        begOut.posL = begPosL;
        begOut.col = begCol;
        output.Append(begOut);
	
        VSOut endOut;
        endOut.pos = mul(MVP, endPosL);
        endOut.posW = mul(M, endPosL);
        endOut.posL = endPosL;
        endOut.col = endCol;
        output.Append(endOut);
    }
}