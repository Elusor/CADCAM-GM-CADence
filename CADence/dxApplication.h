#pragma once
#include "TransformationController.h"
#include "Scene.h"
#include "CameraController.h"
#include "windowApplication.h"
#include "PointSelector.h"
#include "Renderer.h"
#include "CameraRegistry.h"

class DxApplication : public mini::WindowApplication
{
public:
	explicit DxApplication(HINSTANCE hInstance);	

protected:
	int MainLoop() override;

private:
	void Render();
	void InitImguiWindows();
	void Update();
	
	std::unique_ptr<CameraRegistry> m_cameraRegistry;

	// Scene and rendering
	std::unique_ptr<Renderer> m_renderer;
	std::shared_ptr<Scene> m_scene;	

	// Input Managment
	std::unique_ptr<TransformationController> m_transController;
	std::unique_ptr<CameraController> m_camController;		
	std::unique_ptr<PointSelector> m_pSelector;
};
