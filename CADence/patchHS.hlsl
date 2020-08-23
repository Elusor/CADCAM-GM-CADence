#include "ShaderStructs.hlsli"
#include "tesselStructs.hlsli"
#define NUM_CONTROL_POINTS 16

cbuffer TessellationBuffer: register(b0) {
	float uDivisions;
	float vDivisions;
};

// Patch Constant Function
HSOutConst2 CalcHSPatchConstants(
	InputPatch<VSOut, NUM_CONTROL_POINTS> patch,
	uint PatchID : SV_PrimitiveID)
{
	HSOutConst2 Output;

	// Insert code to compute Output here
	Output.EdgeTessFactor[0] = Output.EdgeTessFactor[2] = uDivisions;
	Output.EdgeTessFactor[1] = Output.EdgeTessFactor[3] = uDivisions;//uDivisions;
	Output.InsideTessFactor[0] = Output.InsideTessFactor[1] = uDivisions;
	return Output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
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
	float col = (float)i / 15.f;
	Output.color = patch[i].col;

	return Output;
}
