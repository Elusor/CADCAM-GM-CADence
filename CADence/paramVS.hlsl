#include "ShaderStructs.hlsli"

VSOutParams main(VSInParams i)
{
	VSOutParams o;
	o.pos = float4(0, 0, 0, 0);
	o.params = i.params;
	o.col = float4(i.col, 1.0f);
	
	return o;
}