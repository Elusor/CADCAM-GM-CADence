#pragma once
#include <vector>
#include "Transform.h"

struct BezierCoeffs
{
	DirectX::XMFLOAT3 b0;
	DirectX::XMFLOAT3 b1;
	DirectX::XMFLOAT3 b2;
	DirectX::XMFLOAT3 b3;
};

// Coefficients of derivative
struct DerBezierCoeffs
{
	DirectX::XMFLOAT3 b10;
	DirectX::XMFLOAT3 b21;
	DirectX::XMFLOAT3 b32;	
};

struct SecondDerBezierCoeffs
{
	DirectX::XMFLOAT3 b210;
	DirectX::XMFLOAT3 b321;
};

static class BezierCalculator
{
public:
	static DirectX::XMFLOAT3 CalculateBezier2(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, float t);
	static DirectX::XMFLOAT3 CalculateBezier3(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, float t);
	static DirectX::XMFLOAT3 CalculateBezier3(DerBezierCoeffs coeffs, float t);
	static DirectX::XMFLOAT3 CalculateBezier4(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3, float t);
	static DirectX::XMFLOAT3 CalculateBezier4(BezierCoeffs coeffs, float t);
	static std::vector<DirectX::XMFLOAT3> CalculateBezierDeCasteljau(std::vector<DirectX::XMFLOAT3> knots, int samples);
	static DirectX::XMFLOAT3 CalculateDeBoor4(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3, float t);
	static BezierCoeffs ConvertDeBoorToBezier(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3);
	static DerBezierCoeffs GetDerivativeCoefficients(BezierCoeffs coefficients);
	static DerBezierCoeffs GetDerivativeCoefficients(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3);
	static std::vector<DirectX::XMFLOAT3> GetDerivativeCoefficients(std::vector<DirectX::XMFLOAT3> coefficients);
	static SecondDerBezierCoeffs GetSecondDerivativeCoefficients(BezierCoeffs coefficients);
	static SecondDerBezierCoeffs GetSecondDerivativeCoefficients(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3);
};