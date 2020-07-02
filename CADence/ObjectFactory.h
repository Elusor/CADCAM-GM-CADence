#pragma once
#include <string>
#include "objects.h"
#include "torusGenerator.h"
#include "Node.h"
#include "GroupNode.h"
#include "IParametricSurface.h"

struct HoleData;

class ObjectFactory
{
public:	
	
	std::shared_ptr<Node> CreateBezierSurface(Scene* scene,
		int patchesW, int patchesH, XMFLOAT3 middlePosition,
		bool cylinder = false, float width = 5.0f, float height = 5.0f,
		SurfaceWrapDirection wrapDir = SurfaceWrapDirection::None);

	std::shared_ptr<Node> CreateBezierSurfaceC2(Scene* scene,
		int patchesW, int patchesH, XMFLOAT3 middlePosition,
		bool cylinder = false, float width = 5.0f, float height = 5.0f,
		SurfaceWrapDirection wrapDir = SurfaceWrapDirection::None);

	std::shared_ptr<Node> CreateBezierSurface(
		std::vector<std::vector<std::weak_ptr<Node>>> points,
		int patchesW, int patchesH, SurfaceWrapDirection wrapDir = SurfaceWrapDirection::None);

	std::shared_ptr<Node> CreateBezierSurfaceC2(
		std::vector<std::vector<std::weak_ptr<Node>>> points,
		int patchesW, int patchesH, SurfaceWrapDirection wrapDir = SurfaceWrapDirection::None);

	std::shared_ptr<Node> CreateBezierPatch(Scene* scene, float width = 5.0f, float height = 5.0f);

	std::shared_ptr<Node> CreateBezierPatch(
		std::vector<std::weak_ptr<Node>> top = std::vector<std::weak_ptr<Node>>(),
		std::vector<std::weak_ptr<Node>> bottom = std::vector<std::weak_ptr<Node>>(),
		std::vector<std::weak_ptr<Node>> left = std::vector<std::weak_ptr<Node>>(),
		std::vector<std::weak_ptr<Node>> right = std::vector<std::weak_ptr<Node>>());

	std::shared_ptr<Node> CreateBezierPatchC2(
		std::vector<std::weak_ptr<Node>> top = std::vector<std::weak_ptr<Node>>(),
		std::vector<std::weak_ptr<Node>> bottom = std::vector<std::weak_ptr<Node>>(),
		std::vector<std::weak_ptr<Node>> left = std::vector<std::weak_ptr<Node>>(),
		std::vector<std::weak_ptr<Node>> right = std::vector<std::weak_ptr<Node>>());

	std::shared_ptr<Node> CreateGregoryPatch(
		HoleData hole);

	std::shared_ptr<Node> CreateIntersectionCurve(
		IParametricSurface* qSurface, std::vector<DirectX::XMFLOAT2> qParameters,
		IParametricSurface* pSurface, std::vector<DirectX::XMFLOAT2> pParameters);

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
	int m_gregPatchCounter = 0;
	int m_intersectionCurveCounter = 0;
};