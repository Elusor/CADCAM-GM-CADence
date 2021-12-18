#include "DetailPathsCreationManager.h"

DetailPathsCreationManager::DetailPathsCreationManager(IntersectionFinder* intersectionFinder, Scene* scene)
{
	m_intersectionFinder = intersectionFinder;
	m_scene = scene;
}

void DetailPathsCreationManager::CreateDetailPaths(PathModel* model)
{
	// Create offset surfaces for each object in the model

	// Intersect the models that should be intersected

	// Trim the "single intersection objects"

	// Merge the trimmed ares of the base parts

	// Merge the trimmed areas of the main body
	
}

bool DetailPathsCreationManager::SavePathToFile(const std::vector<DirectX::XMFLOAT3>& positions, std::string name)
{
	return false;
}

void DetailPathsCreationManager::PushInstructionToFile(std::ofstream& file, std::string instructionText, bool lastInstr)
{
}
