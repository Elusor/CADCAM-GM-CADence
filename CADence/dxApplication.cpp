#include "dxApplication.h"
#include <DirectXMath.h>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "mathStructures.h"
#include "torusGenerator.h"
#include "camera.h"
#include "Scene.h"
#include "renderData.h"
#include "ObjectFactory.h"
#include "PointSelector.h"

using namespace mini;
using namespace DirectX;
using namespace std;

void NewFrame();

DxApplication::DxApplication(HINSTANCE hInstance)
	: WindowApplication(hInstance)
{
	m_renderer = unique_ptr<Renderer>(new Renderer(m_window));		
	m_scene = std::shared_ptr<Scene>(new Scene());

	m_camController = unique_ptr<CameraController>(new CameraController(m_renderState->m_camera));
	m_pSelector = std::unique_ptr<PointSelector>(new PointSelector(m_renderState->m_camera));
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
			
			m_renderer->RenderImGUI();

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
	auto renderedItems = m_scene->GetItemsToRender();
	m_renderer->Render(renderedItems);
}

void DxApplication::InitImguiWindows()
{
	m_scene->DrawSceneGUI();
	if (ImGui::CollapsingHeader("Transformations"))
	{
		ImGui::Text("Center trasformations at:");
		ImGui::Spacing();

		ImGui::Checkbox("Center trasformations at cursor", &(m_transController->m_transAroundCursor));
	}
	ImGui::Separator();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void NewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}