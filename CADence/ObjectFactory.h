#pragma once
#include <string>
#include "objects.h"
#include "torusGenerator.h"
#include "Node.h"
#include "GroupNode.h"

enum SurfaceWrapDirection
{
	Width,
	Height
};

class ObjectFactory
{
public:	
	
	std::shared_ptr<Node> CreateBezierSurface(Scene* scene, int width, int height,
		XMFLOAT3 position, bool cylinder = false,
		float sizeW = 5.0f, float sizeH = 5.0f, 
		SurfaceWrapDirection wrapDir = SurfaceWrapDirection::Width);

	std::shared_ptr<Node> CreateBezierSurfaceC2(Scene* scene, int width, int height,
		XMFLOAT3 position, bool cylinder = false,
		float sizeW = 5.0f, float sizeH = 5.0f,
		SurfaceWrapDirection wrapDir = SurfaceWrapDirection::Width);

	std::shared_ptr<Node> CreateBezierPatch(
		Scene* scene,
		std::vector<std::weak_ptr<Node>> top = std::vector<std::weak_ptr<Node>>(),
		std::vector<std::weak_ptr<Node>> bottom = std::vector<std::weak_ptr<Node>>(),
		std::vector<std::weak_ptr<Node>> left = std::vector<std::weak_ptr<Node>>(),
		std::vector<std::weak_ptr<Node>> right = std::vector<std::weak_ptr<Node>>());

	std::shared_ptr<Node> CreateBezierPatchC2(
		Scene* scene,
		std::vector<std::weak_ptr<Node>> top = std::vector<std::weak_ptr<Node>>(),
		std::vector<std::weak_ptr<Node>> bottom = std::vector<std::weak_ptr<Node>>(),
		std::vector<std::weak_ptr<Node>> left = std::vector<std::weak_ptr<Node>>(),
		std::vector<std::weak_ptr<Node>> right = std::vector<std::weak_ptr<Node>>());

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

	void ClearScene();
private:	
	std::vector<std::weak_ptr<Node>> FilterObjectTypes(const type_info& typeId, std::vector<std::weak_ptr<Node>> nodes);
	int m_bezierSurfaceCounter = 0;
	int m_bezierPatchCounter = 0;
	int m_bezierCurveCounter = 0;
	int m_torusCounter = 0;
	int m_pointCounter = 0;
};