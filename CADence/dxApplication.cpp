#include "dxApplication.h"
#include <DirectXMath.h>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "mathStructures.h"
#include "torusGenerator.h"
#include "camera.h"
#include "Scene.h"

using namespace mini;
using namespace DirectX;

DxApplication::DxApplication(HINSTANCE hInstance)
	: WindowApplication(hInstance), m_device(m_window)
{		
	ID3D11Texture2D *temp;
	dx_ptr<ID3D11Texture2D> backTexture;
	m_device.swapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&temp));
	backTexture.reset(temp);

	// Create render target view to be able to write on backBuffer
	m_backBuffer = m_device.CreateRenderTargetView(backTexture);

	// assign viewport to RP
	SIZE wndSize = m_window.getClientSize();	
	Viewport viewport{ wndSize };
	m_device.context()->RSSetViewports(1, &viewport);

	// assign depth buffer to RP
	m_depthBuffer = m_device.CreateDepthStencilView(wndSize);
	auto backBuffer = m_backBuffer.get();
	m_device.context()->OMSetRenderTargets(1, &backBuffer, m_depthBuffer.get());

	const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
	const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");

	m_vertexShader = m_device.CreateVertexShader(vsBytes);
	m_pixelShader = m_device.CreatePixelShader(psBytes);
	
#pragma region set up torus surface object
	
	Torus* t = new Torus();
	t->m_bigR = 8;
	t->m_smallR = 3;	

	m_surObj = t;
	SurfaceParametrizationParams* surParams = &(m_surObj->m_surParams);
	SurfaceVerticesDescription* surDesc= &(m_surObj->m_surVerDesc);	
	
	m_surObj->m_surParams.densityX = 10;
	m_surObj->m_surParams.minDensityX = 3;
	m_surObj->m_surParams.maxDensityX = 30;

	m_surObj->m_surParams.densityY = 10;
	m_surObj->m_surParams.minDensityY = 3;
	m_surObj->m_surParams.maxDensityY = 30;

#pragma endregion

	Torus* torus = reinterpret_cast<Torus*>(m_surObj);
	GetTorusVerticesLineList(torus);

	m_vertexBuffer = m_device.CreateVertexBuffer(surDesc->vertices);
	m_indexBuffer = m_device.CreateIndexBuffer(surDesc->indices);

	std::vector<D3D11_INPUT_ELEMENT_DESC> elements{
		{
			"POSITION", 0,
			DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D11_INPUT_PER_VERTEX_DATA, 0
		},
		{
			"COLOR", 0,
			DXGI_FORMAT_R32G32B32_FLOAT, 0,
			offsetof(VertexPositionColor, color),
			D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};
	m_layout = m_device.CreateInputLayout(elements, vsBytes);	

	m_camera = new Camera(
		XMFLOAT3(0.0f, 0.0f, -30.0f), // camera pos 
		XMFLOAT3(0.0f, 0.0f, 0.0f),  // targer pos 
		XMFLOAT2(0.0f, 0.0f), // yaw, pitch
		viewport.Width,
		viewport.Height,
		45.0f, 2.5f, 250.0f); // fov, zNear, zFar
	
	m_camController = new CameraController(m_camera);

	XMStoreFloat4x4(&m_projMat, m_camera->m_projMat);
	m_cbMVP = m_device.CreateConstantBuffer<XMFLOAT4X4>();

	//Setup imGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(this->m_window.getHandle());
	ImGui_ImplDX11_Init(m_device.m_device.get(), m_device.m_context.get());
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

			Clear();
			Update();
			Render();

			InitImguiWindows();
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());			
			m_device.m_swapChain.get()->Present(0,0);			

		}
	} while (msg.message != WM_QUIT);
	return msg.wParam;
}

void DxApplication::Clear()
{
	// Clear render target
	float clearColor[] = { 0.5f, 0.5f, 1.0f, 1.0f };
	m_device.context()->ClearRenderTargetView(m_backBuffer.get(), clearColor);

	// Clera depth stencil
	m_device.context()->ClearDepthStencilView(m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DxApplication::Update()
{
	D3D11_MAPPED_SUBRESOURCE res;
	XMStoreFloat4x4(&m_viewMat, m_camera->GetViewMatrix());
	XMStoreFloat4x4(&m_modelMat, m_surObj->m_transform.GetModelMatrix());

	XMMATRIX mvp = XMLoadFloat4x4(&m_modelMat) * XMLoadFloat4x4(&m_viewMat) * XMLoadFloat4x4(&m_projMat);
	m_device.context()->Map(m_cbMVP.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	m_device.context()->Unmap(m_cbMVP.get(), 0);
}

void DxApplication::Render()
{		
	m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
	m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);

	// object dependant
	m_device.context()->IASetInputLayout(m_layout.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	
	ID3D11Buffer* cbs[] = { m_cbMVP.get() };
	m_device.context()->VSSetConstantBuffers(0, 1, cbs);
	
	ID3D11Buffer* vbs[] = { m_vertexBuffer.get() };
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };	
	m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	m_device.context()->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);

	m_device.context()->DrawIndexed(m_surObj->m_surVerDesc.indices.size(), 0, 0);
}

void DxApplication::InitImguiWindows()
{		
	bool selectedObjectModified = m_surObj->CreateObjectsImguiSection();

	if (selectedObjectModified)
	{
		Torus* torus = reinterpret_cast<Torus*>(m_surObj);
		GetTorusVerticesLineList(torus);
	}
	Scene* s = new Scene();
	s->DrawSceneHierarchy();
	m_vertexBuffer = m_device.CreateVertexBuffer(m_surObj->m_surVerDesc.vertices);
	m_indexBuffer = m_device.CreateIndexBuffer(m_surObj->m_surVerDesc.indices);
}