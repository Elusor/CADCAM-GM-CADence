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

[maxvertexcount(3)]
void main(
	triangle VSOutParams input[3],
	inout TriangleStream<VSOut> output)
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

    for (int idx = 0; idx < 3; idx++)
    {
        float2 uv = input[idx].params;
        float4 col = input[idx].col;
        
        float4 pos = float4(BezierPatchPoint(points, uv), 1.0f);
        float3 normal = normalize(BezierPatchNormal(points, uv));
        
        VSOut vert;        
        vert.pos = mul(MVP, pos);
        vert.posW = mul(M, pos);
        vert.posL = pos;
        vert.col = float4(normal, 1.0f);
        output.Append(vert);
    }
}