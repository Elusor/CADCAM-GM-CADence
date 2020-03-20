#pragma once
#include "TransformationController.h"
#include "Scene.h"
#include "camera.h"
#include "CameraController.h"
#include "windowApplication.h"
#include "dxDevice.h"
#include "mathStructures.h"
#include "cadStructures.h"
#include <DirectXMath.h>
#include "PointSelector.h"

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

	// TODO [MG] get this out of here
	bool m_isTransAroundCursor;

	std::unique_ptr<TransformationController> m_transController;
	std::unique_ptr<PointSelector> m_pSelector;
	std::shared_ptr<Scene> m_scene;	
	std::unique_ptr<CameraController> m_camController;		
	std::unique_ptr<RenderData> m_renderData;
};
