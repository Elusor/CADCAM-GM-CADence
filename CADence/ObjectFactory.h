#pragma once
#include <string>
#include "objects.h"
#include "torusGenerator.h"
#include "Node.h"
#include "GroupNode.h"
class ObjectFactory
{

public:	

	std::shared_ptr<Node> CreateBezierCurve(
		std::vector<std::weak_ptr<Node>> controlPoints = std::vector<std::weak_ptr<Node>>());

	std::shared_ptr<Node> CreateTorus(
		Transform transform = Transform(),
		std::string name = "Torus",
		float bigR = 8, float smallR = 3,
		float densityX = 10, float densityY = 10,
		float minDensityX = 3, float minDensityY = 3,
		float maxDensityX = 30, float maxDensityY = 30);

	std::shared_ptr<Node> CreateSpawnMarker();

	std::shared_ptr<Node> CreatePoint(Transform transform = Transform());

private:
	int m_bezierCurveCounter = 0;
	int m_torusCounter = 0;
	int m_pointCounter = 0;
};