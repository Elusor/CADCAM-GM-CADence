#include "bezierCalculator.h"
#include "mathUtils.h"

DirectX::XMFLOAT3 BezierCalculator::CalculateBezier2(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, float t)
{
	return WeightedXMFloat3Average(p0, p1, t);
}

DirectX::XMFLOAT3 BezierCalculator::CalculateBezier3(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, float t)
{
	DirectX::XMFLOAT3 t1 = CalculateBezier2(p0, p1, t);
	DirectX::XMFLOAT3 t2 = CalculateBezier2(p1, p2, t);
	return  WeightedXMFloat3Average(t1, t2, t);
}

DirectX::XMFLOAT3 BezierCalculator::CalculateBezier4(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3, float t)
{
	DirectX::XMFLOAT3 t1 = CalculateBezier3(p0, p1, p2, t);
	DirectX::XMFLOAT3 t2 = CalculateBezier3(p1, p2, p3, t);
	return WeightedXMFloat3Average(t1, t2, t);
}

std::vector<DirectX::XMFLOAT3> BezierCalculator::CalculateBezierDeCasteljau(std::vector<DirectX::XMFLOAT3> knots, int samples)
{
	// get degree + 1 points and calculate a point based on them
	
	int curveCount = (knots.size() / 3) + 1;
	float coeff[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	std::vector<DirectX::XMFLOAT3> values;

	for (int curveIdx = 0; curveIdx < curveCount; curveIdx++)
	{

		int index = 3 * curveIdx;
		for (int i = 0; i < samples; i++)
		{
			Transform transform = Transform();
			float t = float(i) / (float)(samples - 1);
			float u = 1.0f - t;

			// calc for 4 pts 
			if (knots.size() - index >= 4)
			{
				values.push_back(CalculateBezier4(knots[index], knots[index + 1], knots[index + 2], knots[index + 3], u));
			}
			else {
				// calc for 3 pts 
				if (knots.size() - index == 3)
				{
					values.push_back(CalculateBezier3(knots[index], knots[index + 1], knots[index + 2], u));
				}
				// calc for 2 pts 
				if (knots.size() - index == 2)
				{
					values.push_back(CalculateBezier2(knots[index], knots[index + 1], u));
				}
				// calc for 1 pt
				if (knots.size() - index == 1)
				{
					values.push_back(knots[index]);
				}
			}					
		}
	}

	return values;
}