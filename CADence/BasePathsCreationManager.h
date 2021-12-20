#pragma once
#include "IntersectionFinder.h"
#include "PathModel.h"
#include "ObjectFactory.h"

#include "SimpleMath.h"
#include "PathUtils.h"

class BasePathsCreationManager
{
public:
	BasePathsCreationManager(IntersectionFinder* intersectionFinder, Scene* scene, float baseHeight);

	void CreateBasePaths(PathModel* model);
private:
	typedef std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3> pointPair;

	IntersectionFinder* m_intersectionFinder;
	Scene* m_scene;
	ObjectFactory m_factory;
	float m_blockBaseHeight;
	float m_blockSafeHeight;
	int m_instructionCounter;
	// block dimensions
	// milling model

	std::vector<DirectX::XMFLOAT2> CalculateOffsetSurfaceIntersections(PathModel* model);
	std::vector<DirectX::XMFLOAT3> IntersectPointsWithVerticalMillLines(const std::vector<DirectX::XMFLOAT3>& outlinePoints);

	void VisualizeCurve(IParametricSurface* surface, const std::vector<DirectX::XMFLOAT2>& params, float distance = -5.F);
	
	std::vector<DirectX::XMFLOAT3> AddBottomPointsLeftToRight(
		const std::vector<pointPair>& bottomPairs,
		const std::vector<LineIntersectionData>& intersectionPoints,
		const std::vector<DirectX::XMFLOAT3>& outlinePoints);

	std::vector<DirectX::XMFLOAT3> AddUpperPointsLeftToRight(
		const std::vector<pointPair>& upperPairs,
		const std::vector<LineIntersectionData>& intersectionPoints,
		const std::vector<DirectX::XMFLOAT3>& outlinePoints);

	void AddAfterPoints(
		const std::vector<pointPair>& afterPairs,
		const bool startFromBottom,
		std::vector<DirectX::XMFLOAT3>& endPoints
	);

	bool SavePathToFile(
		const std::vector<DirectX::XMFLOAT3>& positions,
		const std::vector<DirectX::XMFLOAT3>& prePoints,
		const std::vector<DirectX::XMFLOAT3>& postPoints,
		std::string name);
	void PushInstructionToFile(std::ofstream& file, std::string instructionText, bool lastInstr = false);
};