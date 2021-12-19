#include "DetailPathsCreationManager.h"
#include "ParametricOffsetSurface.h"
#include "IParametricSurface.h"
#include "Scene.h"

DetailPathsCreationManager::DetailPathsCreationManager(IntersectionFinder* intersectionFinder, Scene* scene)
{
	m_intersectionFinder = intersectionFinder;
	m_scene = scene;
}

void DetailPathsCreationManager::CreateDetailPaths(PathModel* model)
{
	float offset = -0.09F; // 0.08 + 0.01 eps

	auto modelObjects = model->GetModelObjects();
	auto bodyObject		= dynamic_cast<IParametricSurface*>(modelObjects[0].lock().get()->m_object.get());
	auto eyeObject		= dynamic_cast<IParametricSurface*>(modelObjects[1].lock().get()->m_object.get());
	auto hairObject		= dynamic_cast<IParametricSurface*>(modelObjects[2].lock().get()->m_object.get());
	auto backFinObject	= dynamic_cast<IParametricSurface*>(modelObjects[3].lock().get()->m_object.get());
	auto sideFinObject	= dynamic_cast<IParametricSurface*>(modelObjects[4].lock().get()->m_object.get());
	auto sideSpikes		= dynamic_cast<IParametricSurface*>(modelObjects[5].lock().get()->m_object.get());

	// Create offset surfaces for each object in the model
	auto bodyOffsetObject		= ParametricOffsetSurface(bodyObject, offset);
	auto eyeOffsetObject		= ParametricOffsetSurface(eyeObject, offset);
	auto hairOffsetObject		= ParametricOffsetSurface(hairObject, offset);
	auto backFinOffsetObject	= ParametricOffsetSurface(backFinObject, offset);
	auto sideFinOffsetObject	= ParametricOffsetSurface(sideFinObject, offset);
	auto sideSpikesOffsetObject = ParametricOffsetSurface(sideSpikes, offset);
	
	// Intersect the models that should be intersected
	auto bodyXbackFinIntersection = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&backFinOffsetObject,
		DirectX::XMFLOAT3(3.8f, 0.5f, -0.5f)
	);

	auto bodyXsideFinIntersection = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&sideFinOffsetObject,
		DirectX::XMFLOAT3(3.8f, 0.5f, -0.5f)
	);

	auto bodyXeyeIntersection = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&eyeOffsetObject,
		DirectX::XMFLOAT3(-5.12f, 1.53f, -0.95f)
	);

	auto bodyXhairIntersection = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&hairOffsetObject,
		DirectX::XMFLOAT3(-3.1f, 4.5f, -0.65f)
	);

	auto bodyXhairIntersection2 = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&hairOffsetObject,
		DirectX::XMFLOAT3(2.37f , 1.5f, -0.225F)
	);

	auto leftUpper = DirectX::XMFLOAT3(-3.25f, 1.66f, -1.62f);
	auto leftLower = DirectX::XMFLOAT3(-3.39f, -0.54f, -1.8f);
	auto rightUpper = DirectX::XMFLOAT3(-1.32f, 1.71f, -1.46f);
	auto rightLower = DirectX::XMFLOAT3(-1.32f, -0.48f, -1.68f);

	auto bodyXsideSpikeLU = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&sideSpikesOffsetObject,
		leftUpper
	);
	auto bodyXsideSpikeLL = m_intersectionFinder->FindIntersectionWithCursor(
		&bodyOffsetObject,
		&sideSpikesOffsetObject,
		leftLower
	);

	//auto bodyXsideSpikeRU = m_intersectionFinder->FindIntersectionWithCursor(
	//	&bodyOffsetObject,
	//	&sideSpikesOffsetObject,
	//	rightUpper
	//);
	//auto bodyXsideSpikeRL = m_intersectionFinder->FindIntersectionWithCursor(
	//	&bodyOffsetObject,
	//	&sideSpikesOffsetObject,
	//	rightLower
	//);

	// Trim the "single intersection objects"

	// Merge the trimmed ares of the base parts

	// Merge the trimmed areas of the main body


	VisualizeCurve(&bodyOffsetObject, bodyXbackFinIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXsideFinIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXeyeIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXhairIntersection);
	VisualizeCurve(&bodyOffsetObject, bodyXhairIntersection2);

	VisualizeCurve(&bodyOffsetObject, bodyXsideSpikeLU);
	VisualizeCurve(&bodyOffsetObject, bodyXsideSpikeLL);	
	// VisualizeCurve(&bodyOffsetObject, bodyXhairIntersection2);
	// VisualizeCurve(&bodyOffsetObject, bodyXhairIntersection2);



}

void DetailPathsCreationManager::VisualizeCurve(IParametricSurface* surface, IntersectionCurveData intersection)
{
	std::vector<ObjectRef> points;
	for (auto param : intersection.surfQParams)
	{
		auto pos = surface->GetPoint(param.x, param.y);
		Transform ptTransform;
		ptTransform.SetPosition(pos);
		ptTransform.Translate(0, 0, 0);
		auto point = m_factory.CreatePoint(ptTransform);
		point->SetIsVirtual(true);
		m_scene->AttachObject(point);
		points.push_back(point);
	}
	m_scene->AttachObject(m_factory.CreateInterpolBezierCurveC2(points));
}

bool DetailPathsCreationManager::SavePathToFile(const std::vector<DirectX::XMFLOAT3>& positions, std::string name)
{
	return false;
}

void DetailPathsCreationManager::PushInstructionToFile(std::ofstream& file, std::string instructionText, bool lastInstr)
{
}
