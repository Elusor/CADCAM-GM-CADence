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


#pragma region FileParsing
	bool SavePathToFile(const std::vector<DirectX::XMFLOAT3>& positions, std::string name);
	void PushInstructionToFile(std::ofstream& file, std::string instructionText, bool lastInstr = false);
#pragma endregion

};
