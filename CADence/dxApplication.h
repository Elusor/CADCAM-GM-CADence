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

	Scene* m_scene;
	Camera* m_camera;
	CameraController* m_camController;		
	RenderData* m_renderData;
};
