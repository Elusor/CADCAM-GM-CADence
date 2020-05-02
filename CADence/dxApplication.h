#pragma once
#include <DirectXMath.h>
#include "TransformationController.h"
#include "Scene.h"
#include "camera.h"
#include "CameraController.h"
#include "windowApplication.h"
#include "dxDevice.h"
#include "vertexStructures.h"
#include "objects.h"
#include "PointSelector.h"
#include "renderState.h"

#include "BackBufferRenderTarget.h"
#include "TextureRenderTarget.h"
#include "IRenderPass.h"
#include "DefaultRenderPass.h"
class DxApplication : public mini::WindowApplication
{
public:
	explicit DxApplication(HINSTANCE hInstance);	

protected:
	int MainLoop() override;

private:
	void RenderPass();
	void Clear();
	void Render();
	void InitImguiWindows();
	void Update();

	std::unique_ptr<TransformationController> m_transController;
	std::unique_ptr<PointSelector> m_pSelector;
	std::shared_ptr<Scene> m_scene;	
	std::unique_ptr<CameraController> m_camController;		
	std::unique_ptr<RenderState> m_renderData;

	std::unique_ptr<DefaultRenderPass> m_defaultPass;

	BackBufferRenderTarget* m_target;
};
