#pragma once
#include <memory>
#include "imgui.h"
class Scene;

class DeletionManager
{
public:
	explicit DeletionManager(std::shared_ptr<Scene> scene);
	void ProcessInput(ImGuiIO& imguiIO);
	
private:
	void DeleteSelectedObjects();
	std::shared_ptr<Scene> m_scene;
};