#pragma once
#include "IntersectionFinder.h"
#include "PathModel.h"
#include "ObjectFactory.h"

#include "SimpleMath.h"

struct LineIntersectionData
{
	int qLineIndex;
	int pLineIndex;
	DirectX::XMFLOAT2 intersectionPoint;
};

class BasePathsCreationManager
{
public:
	BasePathsCreationManager(IntersectionFinder* intersectionFinder, Scene* scene, float baseHeight);

	void CreateBasePaths(PathModel* model);
private:

	IntersectionFinder* m_intersectionFinder;
	Scene* m_scene;
	ObjectFactory m_factory;
	float m_blockSafeHeight;

	// block dimensions
	// milling model

	void CalculateOffsetSurfaceIntersections(PathModel* model);
	void MergeIntersections();
	void SavePathsToFile();

	void VisualizeCurve(IParametricSurface* surface, const std::vector<DirectX::XMFLOAT2>& params);
	std::vector<LineIntersectionData> IntersectCurves(
		const std::vector<DirectX::XMFLOAT2>& params1,
		const std::vector<DirectX::XMFLOAT2>& params2);

	std::pair<bool, DirectX::XMFLOAT2> GetIntersectionPoint(
		const DirectX::SimpleMath::Vector2& beg1,
		const DirectX::SimpleMath::Vector2& end1,
		const DirectX::SimpleMath::Vector2& beg2,
		const DirectX::SimpleMath::Vector2& end2);

};