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

DirectX::XMFLOAT3 BezierCalculator::CalculateBezier3(DerBezierCoeffs coeffs, float t)
{
	return CalculateBezier3(coeffs.b10, coeffs.b21, coeffs.b32, t);
}

DirectX::XMFLOAT3 BezierCalculator::CalculateBezier4(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3, float t)
{
	DirectX::XMFLOAT3 t1 = CalculateBezier3(p0, p1, p2, t);
	DirectX::XMFLOAT3 t2 = CalculateBezier3(p1, p2, p3, t);
	return WeightedXMFloat3Average(t1, t2, t);
}

DirectX::XMFLOAT3 BezierCalculator::CalculateBezier4(BezierCoeffs coeffs, float t)
{
	return CalculateBezier4(coeffs.b0, coeffs.b1, coeffs.b2, coeffs.b3, t);
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

BezierCoeffs BezierCalculator::ConvertDeBoorToBezier(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3)
{
	BezierCoeffs coeffs;

	DirectX::XMFLOAT3 first, second, third, last;

	auto aux1 = (p0 * 1.f / 3.f) + (p1 * 2.f / 3.f);
	auto aux2 = (p2 * 2.f / 3.f) + (p3 * 1.f / 3.f);
	second = (p1 * 2.f / 3.f )+ (p2 * 1.f / 3.f);
	third =  (p1 * 1.f / 3.f )+ (p2 * 2.f / 3.f);
	first = (aux1 + second) * 0.5f;
	last = (third + aux2) * 0.5f;

	coeffs.b0 = first;
	coeffs.b1 = second;
	coeffs.b2 = third;
	coeffs.b3 = last;

	return coeffs;
}

BezierCoeffs BezierCalculator::ConvertDeBoorToBezier(BezierCoeffs coeffs)
{
	return ConvertDeBoorToBezier(coeffs.b0, coeffs.b1, coeffs.b2, coeffs.b3);
}

DerBezierCoeffs BezierCalculator::GetDerivativeCoefficients(BezierCoeffs coefficients)
{
	return GetDerivativeCoefficients(coefficients.b0, coefficients.b1, coefficients.b2, coefficients.b3);
}

// Converts b0 b1 b2 b3 b4 ... to deg(b1-b0), deg(b2-b1), deg(b3-b2), ...
std::vector<DirectX::XMFLOAT3> BezierCalculator::GetDerivativeCoefficients(std::vector<DirectX::XMFLOAT3> coefficients)
{
	std::vector<DirectX::XMFLOAT3> result;

	// Cubic Bezier has 4 coeficcients, Quadratic - 3  and so on
	int degree = coefficients.size() - 1;
	for (int i = 0; i < coefficients.size() - 1; i++) {
		result.push_back(degree * (coefficients[i+1] - coefficients[i]));
	}

	return result;
}

SecondDerBezierCoeffs BezierCalculator::GetSecondDerivativeCoefficients(BezierCoeffs coefficients)
{
	return GetSecondDerivativeCoefficients(coefficients.b0, coefficients.b1, coefficients.b2, coefficients.b3);
}

SecondDerBezierCoeffs BezierCalculator::GetSecondDerivativeCoefficients(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3)
{
	std::vector<DirectX::XMFLOAT3> curve;
	curve.push_back(p0);
	curve.push_back(p1);
	curve.push_back(p2);
	curve.push_back(p3);

	// First derivative
	curve = GetDerivativeCoefficients(curve);
	// Second derivative
	curve = GetDerivativeCoefficients(curve);

	return SecondDerBezierCoeffs{ curve[0], curve[1] };
}

DerBezierCoeffs BezierCalculator::GetDerivativeCoefficients(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3)
{
	DerBezierCoeffs derivatives;
	derivatives.b32 = 3 * (p3 - p2);
	derivatives.b21 = 3 * (p2 - p1);
	derivatives.b10 = 3 * (p1 - p0);
	return derivatives;
}

DirectX::XMFLOAT3 BezierCalculator::CalculateDeBoor4(BezierCoeffs coeffs, float t)
{
	return CalculateDeBoor4(coeffs.b0, coeffs.b1, coeffs.b2, coeffs.b3, t);
}

DirectX::XMFLOAT3 BezierCalculator::CalculateDeBoor4(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3, float t)
{
	auto coeffs = ConvertDeBoorToBezier(p0, p1, p2, p3);
	return CalculateBezier4(coeffs.b0, coeffs.b1, coeffs.b2, coeffs.b3, t);
}
