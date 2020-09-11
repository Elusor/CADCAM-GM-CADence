#include "DeletionManager.h"
#include "Scene.h"

DeletionManager::DeletionManager(std::shared_ptr<Scene> scene)
{
	m_scene = scene;
}

void DeletionManager::ProcessInput(ImGuiIO& imguiIO)
{
	if (ImGui::IsKeyDown('D') && imguiIO.KeyCtrl)
	{
		DeleteSelectedObjects();
	}
}

void DeletionManager::DeleteSelectedObjects()
{
	auto selectedObjects = m_scene->m_selectedNodes;
	for (int i = 0; i < selectedObjects.size(); i++)
	{
		auto weakObj = selectedObjects[i];
		if (auto obj = weakObj.lock())
		{
			if (obj->m_object->GetInUse() == false)
			{
				m_scene->RemoveObject(obj->m_object);
			}
		}
	}
}
