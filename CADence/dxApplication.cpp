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
	m_renderState = unique_ptr<RenderState>(new RenderState(m_window));
	SIZE wndSize = m_window.getClientSize();
	Viewport viewport{ wndSize };
	m_renderState->m_device.context()->RSSetViewports(1, &viewport);

	// init camera
	m_renderState->m_camera = std::shared_ptr<Camera>(
		new Camera(
		XMFLOAT3(0.0f, 0.0f, -30.0f), // camera pos 
		XMFLOAT3(0.0f, 0.0f, 0.0f),  // targer pos 
		XMFLOAT2(0.0f, -0.55f), // yaw, pitch
		viewport.Width,
		viewport.Height,
		DirectX::XM_PIDIV2, 0.5f, 100.0f)); // fov, zNear, zFar

	m_camController = unique_ptr<CameraController>(new CameraController(m_renderState->m_camera));
	m_scene = std::shared_ptr<Scene>(new Scene());

	const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
	const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");
	const auto bezierGsBytes = DxDevice::LoadByteCode(L"bezierGs.cso");
	
	m_renderState->m_vertexShader = m_renderState->m_device.CreateVertexShader(vsBytes);
	m_renderState->m_pixelShader = m_renderState->m_device.CreatePixelShader(psBytes);
	m_renderState->m_bezierGeometryShader = m_renderState->m_device.CreateGeometryShader(bezierGsBytes);

	auto elements = VertexPositionColor::GetInputLayoutElements();
	m_renderState->m_layout = m_renderState->m_device.CreateInputLayout(elements, vsBytes);

	m_renderState->m_cbM = m_renderState->m_device.CreateConstantBuffer<XMFLOAT4X4>();
	m_renderState->m_cbVP = m_renderState->m_device.CreateConstantBuffer<XMFLOAT4X4>();
	m_renderState->m_cbGSData = m_renderState->m_device.CreateConstantBuffer<XMFLOAT4>();

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
#pragma region point selection

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

#pragma endregion
			}
		
			InitImguiWindows();
			Update();
			m_activePass->Execute(m_renderState, m_scene.get());		
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			m_renderState->m_device.m_swapChain.get()->Present(0, 0);
		}
	} while (msg.message != WM_QUIT);
	return msg.wParam;
}

void DxApplication::RenderPass()
{
	Clear();
	Render();
}

void DxApplication::Clear()
{
	// Clear render target
	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_target->ClearRenderTarget(m_renderState->m_device.m_context.get(), m_renderState->m_depthBuffer.get(), 0.2f, 0.2f, 0.2f, 1.0f, 1.0f);
	m_renderState->m_device.context()->ClearRenderTargetView(m_renderState->m_backBuffer.get(), clearColor);

	// Clera depth stencil
	m_renderState->m_device.context()->ClearDepthStencilView(m_renderState->m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DxApplication::Update()
{
	m_scene->UpdateScene();
	m_scene->LateUpdate();
}

void DxApplication::Render()
{
	// Lighting/display style dependant (a little bit object dependant)
	m_renderState->m_device.context()->VSSetShader(m_renderState->m_vertexShader.get(), nullptr, 0);
	m_renderState->m_device.context()->PSSetShader(m_renderState->m_pixelShader.get(), nullptr, 0);

	// object dependant
	m_renderState->m_device.context()->IASetInputLayout(m_renderState->m_layout.get());


	m_scene->RenderScene(m_renderState);
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