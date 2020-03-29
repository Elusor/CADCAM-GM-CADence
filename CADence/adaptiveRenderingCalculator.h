#pragma once
#include <vector>
#include <memory>
#include "Node.h"

static class AdaptiveRenderingCalculator
{
public:
	static unsigned int CalculateAdaptiveSamplesCount(std::vector<std::weak_ptr<Node>> points, std::unique_ptr<RenderState>& renderState);
};