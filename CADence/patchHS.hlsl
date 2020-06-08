#include "ShaderStructs.hlsli"
#include "tesselStructs.hlsli"
#define NUM_CONTROL_POINTS 16

cbuffer TessellationBuffer: register(b0) {
	float uDivisions;
	float vDivisions;
};

// Patch Constant Function
HSOutConst CalcHSPatchConstants(
	InputPatch<VSOut, NUM_CONTROL_POINTS> patch,
	uint PatchID : SV_PrimitiveID)
{
	HSOutConst Output;

	// Insert code to compute Output here
	Output.EdgeTessFactor[0] = uDivisions;
	Output.EdgeTessFactor[1] = 64;//uDivisions;
	//Output.EdgeTessFactor[1] = Output.EdgeTessFactor[3] = 1;//vDivisions;
	//Output.InsideTessFactor[0] = Output.InsideTessFactor[1] = 16;

	return Output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(16)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(64.0f)]
HSOutCP main( 
	InputPatch<VSOut, NUM_CONTROL_POINTS> patch,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HSOutCP Output;
    Output.pos = patch[i].pos;
	Output.posL = patch[i].posL;
	Output.color = patch[i].col;

	return Output;
}
