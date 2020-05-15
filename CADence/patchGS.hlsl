cbuffer pointsData: register(c0) {
	float4x4 xCoords;
	float4x4 yCoords;
	float4x4 zCoords;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
};

[maxvertexcount(3)]
void main(
	triangle float4 input[3] : SV_POSITION, 
	inout TriangleStream< GSOutput > output
)
{
	// TODO: WIP
	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.pos = input[i];
		output.Append(element);
	}
}