#pragma once
#include "imgui.h"
#include "Scene.h"
#include "camera.h"

class SpawnMarkerManager
{
public:

	explicit SpawnMarkerManager(Scene* scene, Camera* camera);
	void ProcessInput(ImGuiIO& io);

private:
	Scene* m_scene;
	Camera* m_camera;
};