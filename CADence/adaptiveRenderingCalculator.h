#pragma once
#include <vector>
#include <memory>
#include "Node.h"

class RenderState;

static class AdaptiveRenderingCalculator
{
public:
	static unsigned int CalculateAdaptiveSamplesCount(std::vector<Object*> points, std::unique_ptr<RenderState>& renderState);
};