#include "ShaderStructs.hlsli"

cbuffer transformations : register(b0) {
	float4 gsData;
}


VSOut Bezier1(VSOut p0, VSOut p1, float t)
{
	VSOut v1;
	v1.pos = p0.pos * (1.0f - t) + p1.pos * t;
	v1.posL = p0.posL * (1.0f - t) + p1.posL * t;
    v1.posW = p0.posW * (1.0f - t) + p1.posW * t;
	v1.col = p0.col * (1.0f - t) + p1.col * t;
	return v1;
}

VSOut Bezier2(VSOut p0, VSOut p1, VSOut p2, float t)
{
	VSOut v1 = Bezier1(p0, p1, t);
	VSOut v2 = Bezier1(p1, p2, t);
	VSOut vr;
	vr.pos = v1.pos * (1.0f - t) + v2.pos * t;
    vr.posL= v1.posL* (1.0f - t) + v2.posL* t;
    vr.posW= v1.posW* (1.0f - t) + v2.posW* t;
	vr.col = v1.col * (1.0f - t) + v2.col * t;
	return vr;
}

VSOut Bezier3(VSOut p0, VSOut p1, VSOut p2, VSOut p3, float t)
{
	VSOut v1 = Bezier2(p0, p1, p2, t);
	VSOut v2 = Bezier2(p1, p2, p3, t);
	VSOut vr;
	vr.pos = v1.pos * (1.0f - t) + v2.pos * t;
    vr.posL = v1.posL * (1.0f - t) + v2.posL * t;
    vr.posW = v1.posW * (1.0f - t) + v2.posW * t;
	vr.col = v1.col * (1.0f - t) + v2.col * t;
	return vr;
}


[instance(20)]
[maxvertexcount(64)]
void main(
	lineadj VSOut input[4] : SV_POSITION,
	uint InstanceID : SV_GSInstanceID,
	inout LineStream<VSOut> output)
{

	int dummyNodesCount = (int)gsData.y;
	int adaptiveRenderCount = (int)gsData.x; // What the adaptive renderer determined

	float instanceStep = 1.0f / 20.0f;
	float curStart = instanceStep * (float)InstanceID;
	float iterStep = instanceStep / adaptiveRenderCount;
	
	// check if any of the nodes are repeating and how many times
	bool b = (input[3].pos) == (input[2].pos);
	if (dummyNodesCount > 0)
	{
		// last node repeats 3 times - linear
		
		if (dummyNodesCount > 1)
		{
			//CalculateLinearPoints();
			for (uint i = 0; i <= adaptiveRenderCount; i++)
			{
				float tParam = curStart + iterStep * (float)i;
				float nextParam = curStart + iterStep * (float)(i + 1);
				VSOut curveVertex = 
					Bezier1( input[0], input[1], tParam);
				output.Append(curveVertex);
			}
		}
		// last node repeart 2 time - quadratic
		else 
		{
			//CalculateQuadraticPoints();
			for (uint i = 0; i <= adaptiveRenderCount; i++)
			{
				float tParam = curStart + iterStep * (float)i;
				float nextParam = curStart + iterStep * (float)(i + 1);
				VSOut curveVertex = Bezier2(
					input[0],
					input[1],
					input[2],
					tParam);

				output.Append(curveVertex);
			}
		}
	}
	else 
	{
		// last node does not repeat - cubic
		//CalculateCubicPoints();
		for (uint i = 0; i <= adaptiveRenderCount; i++)
		{
			float tParam = curStart + iterStep * (float)i;
			float nextParam = curStart + iterStep * (float)(i + 1);
			VSOut curveVertex = Bezier3(
				input[0],
				input[1],
				input[2],
				input[3],
				tParam);

			VSOut nextCurveVertex = Bezier3(
				input[0],
				input[1],
				input[2],
				input[3],
				nextParam);

			output.Append(curveVertex);
		}
	}

}
//
//void CalculateCubicPoints()
//{
//	for (uint i = 0; i < adaptiveRenderCount; i++)
//	{
//		float tParam = curStart + iterStep * (float)i;
//		float nextParam = curStart + iterStep * (float)(i + 1);
//		VSOut curveVertex = Bezier3(
//			input[0],
//			input[1],
//			input[2],
//			input[3],
//			tParam);
//
//		VSOut nextCurveVertex = Bezier3(
//			input[0],
//			input[1],
//			input[2],
//			input[3],
//			nextParam);
//
//		output.Append(curveVertex);
//		output.Append(curveVertex);
//
//		output.Append(nextCurveVertex);
//		output.Append(nextCurveVertex);
//	}
//}
//
//void CalculateQuadraticPoints()
//{
//	for (uint i = 0; i < adaptiveRenderCount; i++)
//	{
//		float tParam = curStart + iterStep * (float)i;
//		float nextParam = curStart + iterStep * (float)(i + 1);
//		VSOut curveVertex = Bezier2(
//			input[0],
//			input[1],
//			input[2],
//			tParam);
//
//		output.Append(curveVertex);
//	}
//}
//
//void CalculateLinearPoints()
//{
//	for (uint i = 0; i < adaptiveRenderCount; i++)
//	{
//		float tParam = curStart + iterStep * (float)i;
//		float nextParam = curStart + iterStep * (float)(i + 1);
//		VSOut curveVertex = Bezier2(
//			input[0],
//			tParam);
//		output.Append(curveVertex);
//	}
//}
