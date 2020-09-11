#include "SpawnMarkerManager.h"

SpawnMarkerManager::SpawnMarkerManager(Scene* scene, Camera* camera)
{
	m_scene = scene;
	m_camera = camera;
}

void SpawnMarkerManager::ProcessInput(ImGuiIO& io)
{	
	//bool lDown = io.MouseDown[1];
	if (io.KeyCtrl && ImGui::IsKeyDown('C'))
	{
		auto mousePos = io.MousePos;
		auto pos = m_camera->GetTargetPos();
		m_scene->SetCursorPosition(pos);
	}
}