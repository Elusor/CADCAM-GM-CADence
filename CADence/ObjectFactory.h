#pragma once
#include <string>
#include "objects.h"
#include "torusGenerator.h"
#include "Node.h"
#include "GroupNode.h"
class ObjectFactory
{

public:	

	std::shared_ptr<Node> CreateBezierPatch(
		std::vector<std::weak_ptr<Node>> top,
		std::vector<std::weak_ptr<Node>> bottom,
		std::vector<std::weak_ptr<Node>> left,
		std::vector<std::weak_ptr<Node>> right,
		Scene* scene);

	std::shared_ptr<Node> CreateInterpolBezierCurveC2(std::vector<std::weak_ptr<Node>> controlPoints = std::vector<std::weak_ptr<Node>>());

	std::shared_ptr<Node> CreateBezierCurveC2(
		std::vector<std::weak_ptr<Node>> controlPoints = std::vector<std::weak_ptr<Node>>(), BezierBasis basis = BezierBasis::BSpline);

	std::shared_ptr<Node> CreateBezierCurve(
		std::vector<std::weak_ptr<Node>> controlPoints = std::vector<std::weak_ptr<Node>>());
	
	std::shared_ptr<Node> CreateSceneGrid(int gridSize);

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
	std::vector<std::weak_ptr<Node>> FilterObjectTypes(const type_info& typeId, std::vector<std::weak_ptr<Node>> nodes);
	int m_bezierPatchCounter = 0;
	int m_bezierCurveCounter = 0;
	int m_torusCounter = 0;
	int m_pointCounter = 0;
};