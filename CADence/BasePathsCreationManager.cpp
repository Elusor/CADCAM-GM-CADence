#include "BasePathsCreationManager.h"
#include "ParametricOffsetSurface.h"
#include "Scene.h"

BasePathsCreationManager::BasePathsCreationManager(IntersectionFinder* intersectionFinder, Scene* scene, float baseHeight)
{
	m_intersectionFinder = intersectionFinder;
	m_blockSafeHeight = baseHeight;
	m_scene = scene;
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

	float offset = -0.5F;

	auto bodyRef = modelObjects[0];
	auto hairRef = modelObjects[2];
	auto tailRef = modelObjects[3];
	auto baseRef = modelObjects[6];
	
	auto bodyParametricObject = dynamic_cast<IParametricSurface*>(bodyRef.lock().get()->m_object.get());
	auto hairParametricObject = dynamic_cast<IParametricSurface*>(hairRef.lock().get()->m_object.get());
	auto tailParametricObject = dynamic_cast<IParametricSurface*>(tailRef.lock().get()->m_object.get());
	auto baseParametricObject = dynamic_cast<IParametricSurface*>(baseRef.lock().get()->m_object.get());

	ParametricOffsetSurface bodyOffsetSurface(bodyParametricObject, offset);
	ParametricOffsetSurface hairOffsetSurface(hairParametricObject, offset);
	ParametricOffsetSurface tailOffsetSurface(tailParametricObject, offset);

	auto bodyCurveUpper = m_intersectionFinder->FindIntersectionWithCursor(&bodyOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(0, 2.6F, 0));
	auto bodyCurveLower = m_intersectionFinder->FindIntersectionWithCursor(&bodyOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(0, -1.78F, 0));
	
	//auto tailCurveUpper = m_intersectionFinder->FindIntersectionWithCursor(&tailOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(0, 4.5F, 3.8));
	auto tailCurveUpper = m_intersectionFinder->FindIntersectionWithCursor(&tailOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(-0.95F, 5.55F, 0)); //better starting point
	auto tailCurveLower = m_intersectionFinder->FindIntersectionWithCursor(&tailOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(4, -5.25, 0));

	auto hairCurve = m_intersectionFinder->FindIntersectionWithCursor(&hairOffsetSurface, baseParametricObject, DirectX::XMFLOAT3(-0.95F, 5.55F, 0));

	VisualizeCurve(baseParametricObject, bodyCurveUpper.surfPParams);
	VisualizeCurve(baseParametricObject, bodyCurveLower.surfPParams);
	
	VisualizeCurve(baseParametricObject, tailCurveUpper.surfPParams);
	VisualizeCurve(baseParametricObject, tailCurveLower.surfPParams);

	VisualizeCurve(baseParametricObject, hairCurve.surfPParams);

}

void BasePathsCreationManager::MergeIntersections()
{
}

void BasePathsCreationManager::SavePathsToFile()
{
}

void BasePathsCreationManager::VisualizeCurve(IParametricSurface* surface, const std::vector<DirectX::XMFLOAT2>& params)
{
	for (auto pt : params)
	{
		auto pos = surface->GetPoint(pt.x, pt.y);
		Transform ptTransform;
		ptTransform.SetPosition(pos);
		ptTransform.Translate(0, 0, -0.15F);
		m_scene->AttachObject(m_factory.CreatePoint(ptTransform));
	}
}
