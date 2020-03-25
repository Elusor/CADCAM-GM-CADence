#pragma once
#include <vector>
#include "Transform.h"

static class BezierCalculator
{
public:
	static std::vector<Transform> CalculateBezierC0Values(std::vector<Transform> knots, int samples);
};