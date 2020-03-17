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

using namespace mini;
using namespace DirectX;

DxApplication::DxApplication(HINSTANCE hInstance)
	: WindowApplication(hInstance)
{
	// init viewport
	m_renderData = new RenderData(m_window);
	SIZE wndSize = m_window.getClientSize();
	Viewport viewport{ wndSize };
	m_renderData->m_device.context()->RSSetViewports(1, &viewport);

	// init camera
	m_camera = new Camera(
		XMFLOAT3(0.0f, 0.0f, -30.0f), // camera pos 
		XMFLOAT3(0.0f, 0.0f, 0.0f),  // targer pos 
		XMFLOAT2(0.0f, 0.0f), // yaw, pitch
		viewport.Width,
		viewport.Height,
		45.0f, 2.5f, 250.0f); // fov, zNear, zFar
	m_renderData->m_camera = m_camera;
	m_camController = new CameraController(m_camera);

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

	m_scene = new Scene();

#pragma region set up torus surface object

	Torus* t = ObjectFactory::CreateTorus("Torus 1");
	auto node = m_scene->AttachObject(t);

	Torus* t2 = ObjectFactory::CreateTorus("Torus 2");
	node->AttachChild(t2);

	for (int i = 3; i < 6; i++)
	{
		char buff[10];
		snprintf(buff, sizeof(buff), "%s %d", "Torus", i);

		std::string name = buff;
		Torus* t3 = ObjectFactory::CreateTorus(name);
		node = m_scene->AttachObject(t3);
	}
	m_scene->m_selectedNode = node;


#pragma endregion
	
	const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
	const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");

	m_renderData->m_vertexShader = m_renderData->m_device.CreateVertexShader(vsBytes);
	m_renderData->m_pixelShader = m_renderData->m_device.CreatePixelShader(psBytes);

	auto elements = VertexPositionColor::GetInputLayoutElements();
	m_renderData->m_layout = m_renderData->m_device.CreateInputLayout(elements, vsBytes);
	m_renderData->m_cbMVP =  m_renderData->m_device.CreateConstantBuffer<XMFLOAT4X4>();

	//Setup imGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(this->m_window.getHandle());
	ImGui_ImplDX11_Init(m_renderData->m_device.m_device.get(),  m_renderData->m_device.m_context.get());
	ImGui::StyleColorsDark();
}

int DxApplication::MainLoop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof msg);
	do
	{
		if (PeekMessage(&msg, nullptr, 0,0, PM_REMOVE))
		{						
			TranslateMessage(&msg);
			DispatchMessage(&msg);					
		}
		else
		{						
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();			
						
			m_camController->ProcessMessage(&ImGui::GetIO());
			InitImguiWindows();

			Clear();		
			Update();
			Render();

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			m_renderData->m_device.m_swapChain.get()->Present(0,0);
		}
	} while (msg.message != WM_QUIT);
	return msg.wParam;
}

void DxApplication::Clear()
{
	// Clear render target
	float clearColor[] = { 0.5f, 0.5f, 1.0f, 1.0f };
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
		ImGui::Text("Filter scene:");
		static ImGuiTextFilter filter;
		filter.Draw();
		// jesli filter jest pusty to wysweitl liste "lisci"  a jesli nie to zrob normalne DrawSceneHierarchy
		if (filter.InputBuf[0] != '\0')
		{
			//filter 
		}
		/*const char* lines[] = { "aaa1.c", "bbb1.c", "ccc1.c", "aaa2.cpp", "bbb2.cpp", "ccc2.cpp", "abc.h", "hello, world" };
		for (int i = 0; i < IM_ARRAYSIZE(lines); i++)
			if (filter.PassFilter(lines[i]))
				ImGui::BulletText("%s", lines[i]);*/
		else
		{
			ImGui::Separator();
			m_scene->DrawSceneHierarchy(); //TODO [MG]: Get selected Node from this somehow				
		}
	}

	if (ImGui::CollapsingHeader("Transformations"))
	{
		ImGui::Text("Center trasformations at:");
		ImGui::Spacing();
		bool isSceneCenter = true;
		bool isObjectCenter = false;
		ImGui::Checkbox("Object center", &isObjectCenter);

		ImGui::Checkbox("Scene center", &isSceneCenter);
	}
}