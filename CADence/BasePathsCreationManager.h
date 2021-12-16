#pragma once
#include "IntersectionFinder.h"
#include "PathModel.h"
#include "ObjectFactory.h"
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
};