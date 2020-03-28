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
	m_renderData = unique_ptr<RenderState>(new RenderState(m_window));
	SIZE wndSize = m_window.getClientSize();
	Viewport viewport{ wndSize };
	m_renderData->m_device.context()->RSSetViewports(1, &viewport);

	// init camera
	m_renderData->m_camera = std::shared_ptr<Camera>(
		new Camera(
		XMFLOAT3(0.0f, 0.0f, -30.0f), // camera pos 
		XMFLOAT3(0.0f, 0.0f, 0.0f),  // targer pos 
		XMFLOAT2(0.0f, 0.0f), // yaw, pitch
		viewport.Width,
		viewport.Height,
		45.0f, 2.5f, 250.0f)); // fov, zNear, zFar

	m_camController = unique_ptr<CameraController>(new CameraController(m_renderData->m_camera));

	// init backbuffer
	ID3D11Texture2D* temp;
	m_renderData->m_device.swapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&temp));
	dx_ptr<ID3D11Texture2D> backTexture;
	backTexture.reset(temp);

	// Create render target view to be able to write on backBuffer
	m_renderData->m_backBuffer = m_renderData->m_device.CreateRenderTargetView(backTexture);

	// assign depth buffer to RP
	m_renderData->m_depthBuffer = m_renderData->m_device.CreateDepthStencilView(wndSize);
	auto backBuffer = m_renderData->m_backBuffer.get();
	m_renderData->m_device.context()->OMSetRenderTargets(1, &backBuffer, m_renderData->m_depthBuffer.get());

	m_scene = std::shared_ptr<Scene>(new Scene());

	const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
	const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");
	const auto bezierGsBytes = DxDevice::LoadByteCode(L"bezierGs.cso");
	
	m_renderData->m_vertexShader = m_renderData->m_device.CreateVertexShader(vsBytes);
	m_renderData->m_pixelShader = m_renderData->m_device.CreatePixelShader(psBytes);
	m_renderData->m_bezierGeometryShader = m_renderData->m_device.CreateGeometryShader(bezierGsBytes);

	auto elements = VertexPositionColor::GetInputLayoutElements();
	m_renderData->m_layout = m_renderData->m_device.CreateInputLayout(elements, vsBytes);
	m_renderData->m_cbMVP = m_renderData->m_device.CreateConstantBuffer<XMFLOAT4X4>();

	m_pSelector = std::unique_ptr<PointSelector>(new PointSelector(m_renderData->m_camera));

	m_transController = std::unique_ptr<TransformationController>(new TransformationController(m_scene));

	//Setup imGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(this->m_window.getHandle());
	ImGui_ImplDX11_Init(m_renderData->m_device.m_device.get(), m_renderData->m_device.m_context.get());
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
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

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

			Clear();
			Update();
			Render();

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			m_renderData->m_device.m_swapChain.get()->Present(0, 0);
		}
	} while (msg.message != WM_QUIT);
	return msg.wParam;
}

void DxApplication::Clear()
{
	// Clear render target
	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_renderData->m_device.context()->ClearRenderTargetView(m_renderData->m_backBuffer.get(), clearColor);

	// Clera depth stencil
	m_renderData->m_device.context()->ClearDepthStencilView(m_renderData->m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DxApplication::Update()
{
	m_scene->UpdateSelectedNode();
}

void DxApplication::Render()
{
	// Lighting/display style dependant (a little bit object dependant)
	m_renderData->m_device.context()->VSSetShader(m_renderData->m_vertexShader.get(), nullptr, 0);
	m_renderData->m_device.context()->PSSetShader(m_renderData->m_pixelShader.get(), nullptr, 0);

	// object dependant
	m_renderData->m_device.context()->IASetInputLayout(m_renderData->m_layout.get());


	m_scene->RenderScene(m_renderData);
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
	if (ImGui::CollapsingHeader("Transformations"))
	{
		ImGui::Text("Center trasformations at:");
		ImGui::Spacing();
		
		ImGui::Checkbox("Center trasformations at cursor", &(m_transController->m_transAroundCursor));
	}
	ImGui::Separator();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

}