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

	std::vector<DirectX::XMFLOAT3> VisualizeCurve(IParametricSurface* surface, const std::vector<DirectX::XMFLOAT2>& params);

#pragma region FileParsing
	bool SavePathToFile(const std::vector<DirectX::XMFLOAT3>& positions, std::string name);
	void PushInstructionToFile(std::ofstream& file, std::string instructionText, bool lastInstr = false);
#pragma endregion

	std::vector<DirectX::XMFLOAT2> NormalizeParameters(const std::vector<DirectX::XMFLOAT2>& intersectionCurve, IParametricSurface* surface);
	std::vector<DirectX::XMFLOAT2> DenormalizeParameters(const std::vector<DirectX::XMFLOAT2>& intersectionCurve, IParametricSurface* surface);
	
	std::vector<DirectX::XMFLOAT2> PrepareBackFin(const std::vector<DirectX::XMFLOAT2>& bodyXbackFinIntersectionCurve);
	std::vector<DirectX::XMFLOAT2> PrepareSideFin(const std::vector<DirectX::XMFLOAT2>& intersectionParams);
	std::vector<DirectX::XMFLOAT2> PrepareEye(const std::vector<DirectX::XMFLOAT2>& intersectionParams);
	std::vector<DirectX::XMFLOAT2> PrepareBody(
		const std::vector<DirectX::XMFLOAT2>& cutoffTop,
		const std::vector<DirectX::XMFLOAT2>& cutoffBot,
		const std::vector<DirectX::XMFLOAT2>& intersectionParamSideFin,
		const std::vector<DirectX::XMFLOAT2>& intersectionParamBackFin,
		const std::vector<DirectX::XMFLOAT2>& intersectionParamEye,
		const std::vector<DirectX::XMFLOAT2>& intersectionHair1,
		const std::vector<DirectX::XMFLOAT2>& intersectionHair2,
		const std::vector<DirectX::XMFLOAT2>& intersectionSideSpikes);
	std::vector<DirectX::XMFLOAT2> PrepareHair(
		const std::vector<DirectX::XMFLOAT2>& cutoffTop,
		const std::vector<DirectX::XMFLOAT2>& cutoffBot,
		const std::vector<DirectX::XMFLOAT2>& intersectionHair1,
		const std::vector<DirectX::XMFLOAT2>& intersectionHair2);

	std::vector<DirectX::XMFLOAT2> PrepareHole(
		const std::vector<DirectX::XMFLOAT2>& intersectionHair,
		const std::vector<DirectX::XMFLOAT2>& intersectionBody,
		const std::vector<DirectX::XMFLOAT2>& intersectionBackFin);

	std::vector<DirectX::XMFLOAT3> RemoveSpecialPoints(const std::vector<DirectX::XMFLOAT3>& endPathPoints);
};