#pragma once
#include <vector>
#include "Transform.h"

static class BezierCalculator
{
public:
	static std::vector<DirectX::XMFLOAT3> CalculateBezierDeCasteljau(std::vector<DirectX::XMFLOAT3> knots, int samples);
};