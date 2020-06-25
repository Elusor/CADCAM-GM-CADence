#pragma once
#include <vector>
#include "Transform.h"

static class BezierCalculator
{
public:
	static DirectX::XMFLOAT3 CalculateBezier2(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, float t);
	static DirectX::XMFLOAT3 CalculateBezier3(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, float t);
	static DirectX::XMFLOAT3 CalculateBezier4(DirectX::XMFLOAT3 p0, DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3, float t);
	static std::vector<DirectX::XMFLOAT3> CalculateBezierDeCasteljau(std::vector<DirectX::XMFLOAT3> knots, int samples);
};