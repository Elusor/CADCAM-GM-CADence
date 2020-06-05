#include "dxApplication.h"
#include <DirectXMath.h>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "vertexStructures.h"
#include "torusGenerator.h"
#include "camera.h"
#include "Scene.h"
#include "renderState.h"
#include "ObjectFactory.h"
#include "PointSelector.h"

using namespace mini;
using namespace DirectX;
using namespace std;

DxApplication::DxApplication(HINSTANCE hInstance)
	: WindowApplication(hInstance)
{
	// init viewport
	SIZE wndSize = m_window.getClientSize();
	Viewport viewport{ wndSize };	

	// init camera
	std::shared_ptr<Camera> camera = make_shared<Camera>(XMFLOAT3(0.0f, 0.0f, -30.0f), // camera pos 
		XMFLOAT3(0.0f, 0.0f, 0.0f),  // targer pos 
		XMFLOAT2(0.0f, -0.55f), // yaw, pitch
		viewport.Width,
		viewport.Height,
		DirectX::XM_PIDIV2, 0.5f, 150); // fov, zNear, zFar		
			
	m_camController = make_unique<CameraController>(camera);
	m_scene = make_shared<Scene>();
	m_renderState = make_unique<RenderState>(m_window, viewport, camera);

	m_pSelector = std::unique_ptr<PointSelector>(new PointSelector(m_renderState->m_camera));
	m_transController = std::unique_ptr<TransformationController>(new TransformationController(m_scene));
	
	//// RENDER PASS
	m_defPass = new DefaultRenderPass(m_renderState, wndSize);
	m_stereoPass = new StereoscopicRenderPass(m_renderState, wndSize);
	m_activePass = m_defPass;
	////

	//Setup imGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(this->m_window.getHandle());
	ImGui_ImplDX11_Init(m_renderState->m_device.m_device.get(), m_renderState->m_device.m_context.get());
	ImGui::StyleColorsDark();
}

int DxApplication::MainLoop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof msg);
	do
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (m_stereoChanged)
			{
				m_isStereo = !m_isStereo;
				if (m_isStereo)
				{
					m_activePass = m_stereoPass;
				}
				else{
					m_activePass = m_defPass;
				}
				m_stereoChanged = false;
			}

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			m_scene->ClearModifiedTag();

			m_transController->ProcessInput(ImGui::GetIO());
			if (m_transController->IsTransforming() == false) {
				m_camController->ProcessMessage(&ImGui::GetIO());
				m_pSelector->ProcessInput(m_scene, m_window.getClientSize());
				
			}
		
			InitImguiWindows();
			Update();
			m_activePass->Execute(m_renderState, m_scene.get());		
			m_pSelector->DrawSelectionWindow(m_renderState, m_window.getClientSize());
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			m_renderState->m_device.m_swapChain.get()->Present(0, 0);
		}
	} while (msg.message != WM_QUIT);
	return msg.wParam;
}

void DxApplication::Update()
{
	m_scene->UpdateScene();
	m_scene->LateUpdate();
}

void DxApplication::InitImguiWindows()
{

	if (ImGui::CollapsingHeader("Hierarchy"))
	{		
		m_scene->DrawSceneHierarchy();
		ImGui::Spacing();
	}
	if (ImGui::CollapsingHeader("Cursor"))
	{
		m_scene->m_spawnMarker->CreateParamsGui();	
	}
	if (ImGui::CollapsingHeader("Grid"))
	{
		m_scene->m_grid->CreateParamsGui();
	}
	if (ImGui::CollapsingHeader("Transformations"))
	{
		ImGui::Text("Center trasformations at:");
		ImGui::Spacing();

		ImGui::Checkbox("Center trasformations at cursor", &(m_transController->m_transAroundCursor));
	}
	if (ImGui::CollapsingHeader("Stereoscopy"))
	{
		if (m_isStereo) {

			StereoscopicRenderPass* stereo = static_cast<StereoscopicRenderPass*>(m_stereoPass);

			if (ImGui::Button("Disable stereoscopy"))
			{
				m_stereoChanged = true;
			}
			if (ImGui::DragFloat("Focus plane distance", &(stereo->m_focusPlaneDistance), .5f, 0.5f, 100.f));
			if (ImGui::DragFloat("Eye distance", &(stereo->m_eyeDistance), 0.05f, 0.1f, 2.f));
		}
		else {
			if (ImGui::Button("Enable stereoscopy"))
			{
				m_stereoChanged = true;
			}
		}
	}	

	ImGui::Separator();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

}