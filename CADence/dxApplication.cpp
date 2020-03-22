#include "dxApplication.h"
#include <DirectXMath.h>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "geometryRep.h"
#include "torusGenerator.h"
#include "camera.h"
#include "Scene.h"
#include "ObjectFactory.h"
#include "PointSelector.h"
#include "GlobalRenderState.h"
using namespace mini;
using namespace DirectX;
using namespace std;

void NewFrame();

DxApplication::DxApplication(HINSTANCE hInstance)
	: WindowApplication(hInstance)
{
	CameraRegistry::currentCamera = std::shared_ptr<Camera>(
		new Camera(
			DirectX::XMFLOAT3(0.0f, 0.0f, -30.0f), // camera pos 
			DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),  // targer pos 
			DirectX::XMFLOAT2(0.0f, 0.0f), // yaw, pitch
			m_window.getClientSize().cx,
			m_window.getClientSize().cy,
			45.0f, 2.5f, 250.0f)); // fov, zNear, zFar

	m_renderer = unique_ptr<Renderer>(new Renderer(m_window));		
	m_scene = std::shared_ptr<Scene>(new Scene());

	m_camController = unique_ptr<CameraController>(new CameraController());
	m_pSelector = std::unique_ptr<PointSelector>(new PointSelector());
	m_transController = std::unique_ptr<TransformationController>(new TransformationController(m_scene));
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
			NewFrame();



#pragma region input
			m_transController->ProcessInput(ImGui::GetIO());
			if (m_transController->IsTransforming() == false) {
				m_camController->ProcessMessage(&ImGui::GetIO());

				bool lDown = ImGui::GetIO().MouseDown[0];

				if (lDown && !ImGui::GetIO().WantCaptureMouse)
				{
					auto pos = ImGui::GetIO().MousePos;
					auto selectedNode = m_pSelector->GetNearestPoint(pos.x, pos.y, m_scene->m_nodes, m_window.getClientSize().cx, m_window.getClientSize().cy, 50);
					if (auto node = selectedNode.lock())
					{
						for (int i = 0; i < m_scene->m_selectedNodes.size(); i++)
						{
							if (auto nod = m_scene->m_selectedNodes[i].lock())
							{
								nod->m_isSelected = false;
							}
						}
						m_scene->m_selectedNodes.clear();

						node->m_isSelected = true;
						m_scene->m_selectedNodes.push_back(selectedNode);
					}
				}
			}
#pragma endregion

			//m_scene->DrawSceneGUI();
			InitImguiWindows();

			Update();
			Render();
			
			GlobalRenderState::RenderImGUI();

		}
	} while (msg.message != WM_QUIT);
	return msg.wParam;
}

void DxApplication::Update()
{
	m_scene->UpdateSelectedNodes();
}

void DxApplication::Render()
{
	m_scene->RenderScene();
}

void DxApplication::InitImguiWindows()
{
	m_scene->DrawSceneGUI();
	// Object Transformer GUI
	m_transController->DrawGUI();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void NewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}