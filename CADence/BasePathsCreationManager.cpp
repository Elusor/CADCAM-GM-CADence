#include "BasePathsCreationManager.h"
#include "ParametricOffsetSurface.h"

BasePathsCreationManager::BasePathsCreationManager(IntersectionFinder* intersectionFinder, float baseHeight)
{
	m_intersectionFinder = intersectionFinder;
	m_blockSafeHeight = baseHeight;
}

void BasePathsCreationManager::CreateBasePaths(PathModel* model)
{
	// 1. Create proper intersections of the model with the non offset surfaces
	CalculateOffsetSurfaceIntersections(model);
	// 2. Merge intersection lines
	MergeIntersections();
	// 3. Save paths to file
	SavePathsToFile();
}

void BasePathsCreationManager::CalculateOffsetSurfaceIntersections(PathModel* model)
{
	auto modelObjects = model->GetModelObjects();

	auto bodyRef = modelObjects[0];
	auto baseRef = modelObjects[6];
	
	auto bodyParametricObject = dynamic_cast<IParametricSurface*>(bodyRef.lock().get()->m_object.get());
	auto baseParametricObject = dynamic_cast<IParametricSurface*>(baseRef.lock().get()->m_object.get());

	ParametricOffsetSurface bodyOffsetSurface(bodyParametricObject, 0.5F);

	m_intersectionFinder->FindIntersectionWithCursor(bodyRef, baseRef, DirectX::XMFLOAT3(0, 2.6F, 0));
	m_intersectionFinder->FindIntersectionWithCursor(bodyRef, baseRef, DirectX::XMFLOAT3(0, -1.78F, 0));
}

void BasePathsCreationManager::MergeIntersections()
{
}

void BasePathsCreationManager::SavePathsToFile()
{
}
