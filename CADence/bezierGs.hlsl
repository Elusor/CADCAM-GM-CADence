#include "ShaderStructs.hlsli"

[maxvertexcount(3)]
void main(
	line VSOut input[2] : SV_POSITION, 
	inout LineStream<VSOut> output)
{
	VSOut middleVertex;

	middleVertex.pos = input[0].pos + input[1].pos;
	middleVertex.col = float4(1.0f, 0.0f, 0.0f, 1.0f);

	output.Append(input[0]);
	output.Append(middleVertex);
	output.Append(input[1]);

	//for (uint i = 0; i < 3; i++)
	//{
	//	GSOutput element;
	//	element.pos = input[i];
	//	output.Append(element);
	//}
}