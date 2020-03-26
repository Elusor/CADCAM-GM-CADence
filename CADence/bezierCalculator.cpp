#include "bezierCalculator.h"

Transform CalculateBezier2(Transform p0, Transform p1, float t);
Transform CalculateBezier3(Transform p0, Transform p1, Transform p2, float t);
Transform CalculateBezier4(Transform p0, Transform p1, Transform p2, Transform p3, float t);
Transform WeightedTransformAverage(Transform p0, Transform p1, float t);
DirectX::XMFLOAT3 XMF3SUM(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2);
DirectX::XMFLOAT3 XMFloat3TimesFloat(DirectX::XMFLOAT3 vec, float val);

std::vector<Transform> BezierCalculator::CalculateBezierC0Values(std::vector<Transform> knots, int samples)
{
	// get degree + 1 points and calculate a point based on them
	
	int curveCount = (knots.size() / 3) + 1;
	float coeff[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	std::vector<Transform> values;

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

Transform CalculateBezier2(Transform p0, Transform p1, float t)
{	
	return WeightedTransformAverage(p0,p1,t);
}

Transform CalculateBezier3(Transform p0, Transform p1, Transform p2, float t)
{
	Transform t1 = CalculateBezier2(p0, p1,t);
	Transform t2 = CalculateBezier2(p1, p2,t);
	return  WeightedTransformAverage(t1, t2, t);
}

Transform CalculateBezier4(Transform p0, Transform p1, Transform p2, Transform p3, float t)
{
	Transform t1 = CalculateBezier3(p0, p1, p2,t);
	Transform t2 = CalculateBezier3(p1, p2, p3,t);
	return WeightedTransformAverage(t1, t2, t);
}

// t * p0 + (1-t) * p1
Transform WeightedTransformAverage(Transform p0, Transform p1, float t)
{
	return Transform(
		XMF3SUM(XMFloat3TimesFloat(p0.GetPosition(), t), XMFloat3TimesFloat(p1.GetPosition(), 1.0f - t)),
		XMF3SUM(XMFloat3TimesFloat(p0.GetRotation(), t), XMFloat3TimesFloat(p1.GetRotation(), 1.0f - t)),
		XMF3SUM(XMFloat3TimesFloat(p0.GetScale(), t), XMFloat3TimesFloat(p1.GetScale(), 1.0f - t)));
}

DirectX::XMFLOAT3 XMF3SUM(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2)
{
	return DirectX::XMFLOAT3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

DirectX::XMFLOAT3 XMFloat3TimesFloat(DirectX::XMFLOAT3 vec, float val)
{
	DirectX::XMFLOAT3 res = DirectX::XMFLOAT3(
		vec.x * val,
		vec.y * val,
		vec.z * val);

	return res;
}