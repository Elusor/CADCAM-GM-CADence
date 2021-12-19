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
		DirectX::XMFLOAT3(3.8F, 0.5F, -0.5F)
	);

	std::vector<ObjectRef> points;
	for (auto param : bodyXbackFinIntersection.surfQParams)
	{
		auto pos = bodyOffsetObject.GetPoint(param.x, param.y);
		Transform ptTransform;
		ptTransform.SetPosition(pos);
		ptTransform.Translate(0, 0, 0);
		auto point = m_factory.CreatePoint(ptTransform);
		point->SetIsVirtual(true);
		m_scene->AttachObject(point);
		points.push_back(point);
	}

	auto sideWithBody = DirectX::XMFLOAT3(3.8F, 0.5F, -0.5F);
	auto eyeWithBody = DirectX::XMFLOAT3(-5.12F, 1.53F, -0.95F);

	auto x = 2;
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
