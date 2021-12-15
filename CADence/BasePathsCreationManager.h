#pragma once
#include "IntersectionFinder.h"
#include "PathModel.h"

class BasePathsCreationManager
{
public:
	BasePathsCreationManager(IntersectionFinder* intersectionFinder, float baseHeight);

	void CreateBasePaths(PathModel* model);
private:

	IntersectionFinder* m_intersectionFinder;

	float m_blockSafeHeight;

	// block dimensions
	// milling model

	void CalculateOffsetSurfaceIntersections(PathModel* model);
	void MergeIntersections();
	void SavePathsToFile();
};