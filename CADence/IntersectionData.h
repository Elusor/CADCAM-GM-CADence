#pragma once
#include <xmemory>
#include "Node.h"
#include "IntersectionCurve.h"
#include "IntersectedSurface.h"

struct IntersectionData
{
	IntersectionData() {};
	IntersectionData(
		std::weak_ptr<Node> _intersectionCurve, IntersectedSurface _affectedSurface) :
		intersectionCurve(_intersectionCurve), affectedSurface(_affectedSurface) {};

	std::weak_ptr<Node> intersectionCurve;
	IntersectedSurface affectedSurface;
};