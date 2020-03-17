#pragma once
#include "Scene.h"
#include "camera.h"
#include "CameraController.h"
#include "windowApplication.h"
#include "dxDevice.h"
#include "mathStructures.h"
#include "cadStructures.h"
#include <DirectXMath.h>

class DxApplication : public mini::WindowApplication
{
public:
	explicit DxApplication(HINSTANCE hInstance);	

protected:
	int MainLoop() override;

private:
	void Clear();
	void Render();
	void InitImguiWindows();
	void Update();

	std::unique_ptr<Scene> m_scene;	
	std::unique_ptr<CameraController> m_camController;		
	std::unique_ptr<RenderData> m_renderData;
};
