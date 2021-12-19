#pragma once
#include "PathModel.h"
#include "IntersectionFinder.h"
#include "ObjectFactory.h"

class DetailPathsCreationManager
{
public:
	DetailPathsCreationManager(
		IntersectionFinder* intersectionFinder,
		Scene* scene
	);

	void CreateDetailPaths(PathModel* model);

private:


	IntersectionFinder* m_intersectionFinder;
	Scene* m_scene;
	ObjectFactory m_factory;
	float m_blockBaseHeight;
	float m_blockSafeHeight;
	int m_instructionCounter;

	void VisualizeCurve(IParametricSurface* surface, const std::vector<DirectX::XMFLOAT2>& params);

#pragma region FileParsing
	bool SavePathToFile(const std::vector<DirectX::XMFLOAT3>& positions, std::string name);
	void PushInstructionToFile(std::ofstream& file, std::string instructionText, bool lastInstr = false);
#pragma endregion

	std::vector<DirectX::XMFLOAT2> NormalizeParameters(const std::vector<DirectX::XMFLOAT2>& intersectionCurve, IParametricSurface* surface);
	std::vector<DirectX::XMFLOAT2> DenormalizeParameters(const std::vector<DirectX::XMFLOAT2>& intersectionCurve, IParametricSurface* surface);
	std::vector<DirectX::XMFLOAT2> PrepareBackFin(const std::vector<DirectX::XMFLOAT2>& bodyXbackFinIntersectionCurve);

};