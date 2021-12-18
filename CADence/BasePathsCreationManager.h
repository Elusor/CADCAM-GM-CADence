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
	
	std::vector<LineIntersectionData> IntersectCurves(
		const std::vector<DirectX::XMFLOAT2>& params1,
		const std::vector<DirectX::XMFLOAT2>& params2);

	std::vector<LineIntersectionData> IntersectCurves(
		const std::vector<DirectX::XMFLOAT3>& params1,
		const std::vector<DirectX::XMFLOAT3>& params2);

	std::pair<bool, DirectX::XMFLOAT2> GetIntersectionPoint(
		const DirectX::SimpleMath::Vector2& beg1,
		const DirectX::SimpleMath::Vector2& end1,
		const DirectX::SimpleMath::Vector2& beg2,
		const DirectX::SimpleMath::Vector2& end2);

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

	std::vector<DirectX::XMFLOAT3> ExtractSegmentFromOutline(const std::vector<DirectX::XMFLOAT3>& outline, int line1, int line2);



	bool SavePathToFile(
		const std::vector<DirectX::XMFLOAT3>& positions,
		const std::vector<DirectX::XMFLOAT3>& prePoints,
		const std::vector<DirectX::XMFLOAT3>& postPoints,
		std::string name);
	void PushInstructionToFile(std::ofstream& file, std::string instructionText, bool lastInstr = false);
};