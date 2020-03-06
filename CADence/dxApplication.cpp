#include "dxApplication.h"
#include <DirectXMath.h>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "mathStructures.h"
#include "torusGenerator.h"
using namespace mini;
using namespace DirectX;

void InitImguiWindows();

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

	// load shaders and assign them to the device 
	const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
	const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");

	m_vertexShader = m_device.CreateVertexShader(vsBytes);
	m_pixelShader = m_device.CreatePixelShader(psBytes);

	// Generate vertices and create vertex buffer and bind it to Input Layout
	SurfaceParametrizationParams parameters { 20, 10 };
	SurfaceVerticesDescription* verticesDesc = GetTorusVerticesLineList(5, 3, parameters);

	m_vertexBuffer = m_device.CreateVertexBuffer(verticesDesc->vertices);
	m_indexBuffer = m_device.CreateIndexBuffer(verticesDesc->indices);

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

	//Add constant buffer with MVP matrix
	XMStoreFloat4x4(&m_modelMat, XMMatrixIdentity());
	XMStoreFloat4x4(&m_viewMat, XMMatrixRotationX(XMConvertToRadians(-30))*XMMatrixTranslation(0.0f,0.0f,30.0f));
	XMStoreFloat4x4(&m_projMat, XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45),
		static_cast<float> (wndSize.cx) / wndSize.cy,
		0.1f,
		100.0f));
	m_cbMVP = m_device.CreateConstantBuffer<XMFLOAT4X4>();


	//Setup imGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(this->m_window.getHandle());
	ImGui_ImplDX11_Init(m_device.m_device.get(), m_device.m_context.get());
	ImGui::StyleColorsDark();

}

std::vector<VertexPositionColor> DxApplication::CreateCubeVertices()
{
	return {

		// 2 -- 3
		// |    |
		// 0 -- 1
		// position, color
		// Front face 		
		{ {-0.5f,-0.5f,-0.5f}, { 1.0f, 0.0f, 0.0f } },	//0
		{ { 0.5f,-0.5f,-0.5f}, { 1.0f, 0.0f, 0.0f } },	//1
		{ {-0.5f, 0.5f,-0.5f}, { 1.0f, 0.0f, 0.0f } },	//2
		{ { 0.5f, 0.5f,-0.5f}, { 1.0f, 0.0f, 0.0f } },	//3
		// Back face
		{ { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.0f } },	//4
		{ {-0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.0f } },	//5
		{ { 0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f, 0.0f } },	//6
		{ {-0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f, 0.0f } },	//7
		
		// Upper face
		{ {-0.5f, 0.5f,-0.5f}, { 0.0f, 0.5f, 0.5f } },	//8
		{ { 0.5f, 0.5f,-0.5f}, { 0.0f, 0.5f, 0.5f } },	//9
		{ {-0.5f, 0.5f, 0.5f}, { 0.0f, 0.5f, 0.5f } },	//10
		{ { 0.5f, 0.5f, 0.5f}, { 0.0f, 0.5f, 0.5f } },	//11
		
		// Lower Face
		{ {-0.5f,-0.5f, 0.5f}, { 0.5f, 0.0f, 0.5f } },	//12
		{ { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.0f, 0.5f } },	//13
		{ {-0.5f,-0.5f,-0.5f}, { 0.5f, 0.0f, 0.5f } },	//14
		{ { 0.5f,-0.5f,-0.5f}, { 0.5f, 0.0f, 0.5f } },	//15
		
		// Left face
		{ {-0.5f,-0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f } },	//16
		{ {-0.5f,-0.5f,-0.5f}, { 0.0f, 0.0f, 1.0f } },	//17
		{ {-0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f } },	//18
		{ {-0.5f, 0.5f,-0.5f}, { 0.0f, 0.0f, 1.0f } },	//19
		
		// Right face
		{ {0.5f,-0.5f,-0.5f}, { 0.0f, 1.0f, 0.0f } },	//20
		{ {0.5f,-0.5f, 0.5f}, { 0.0f, 1.0f, 0.0f } },	//21
		{ {0.5f, 0.5f,-0.5f}, { 0.0f, 1.0f, 0.0f } },	//22
		{ {0.5f, 0.5f, 0.5f}, { 0.0f, 1.0f, 0.0f } }	//23
	};
}

std::vector<unsigned short> DxApplication::CreateCubeIndices()
{
	return {
		0,3,1,
		0,2,3,

		4,7,5,
		4,6,7,

		8,11,9,
		8,10,11,

		12,15,13,
		12,14,15,

		16,19,17,
		16,18,19,

		20,23,21,
		20,22,23
	};
}

int DxApplication::MainLoop()
{
	MSG msg;
	//PeekMessage doesn't change MSG if there are no messages to be recieved.
	//However unlikely the case may be, that the first call to PeekMessage
	//doesn't find any messages, msg is zeroed out to make sure loop condition
	//isn't reading unitialized values.
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
			ImGui::GetIO().WantCaptureMouse = true;
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();

			ImGui::NewFrame();
			InitImguiWindows();
			ImGui::End();
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());			
			m_device.m_swapChain.get()->Present(0,0);
			Update();
			Render();
		}
	} while (msg.message != WM_QUIT);
	return msg.wParam;
}

void DxApplication::Update()
{
	XMStoreFloat4x4(&m_modelMat, XMLoadFloat4x4(&m_modelMat) * XMMatrixRotationY(0.0001f));
	D3D11_MAPPED_SUBRESOURCE res;

	m_device.context()->Map(m_cbMVP.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	XMMATRIX mvp = XMLoadFloat4x4(&m_modelMat) * XMLoadFloat4x4(&m_viewMat) * XMLoadFloat4x4(&m_projMat);
	memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	m_device.context()->Unmap(m_cbMVP.get(), 0);

}

void DxApplication::Render()
{
	// Clear the back buffer
	float clearColor[] = { 0.5f, 0.5f, 1.0f, 1.0f };
	m_device.context()->ClearRenderTargetView(m_backBuffer.get(), clearColor);

	//Assing all private variables to the RP
	m_device.context()->ClearDepthStencilView(m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	
	//Set shaders
	
	m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
	m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
	ID3D11Buffer* cbs[] = { m_cbMVP.get() };
	m_device.context()->VSSetConstantBuffers(0, 1, cbs);
	//bind input layout and topology
	m_device.context()->IASetInputLayout(m_layout.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	//set vertex buffer
	ID3D11Buffer* vbs[] = { m_vertexBuffer.get() };
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };
	
	m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	m_device.context()->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);

	m_device.context()->DrawIndexed(800, 0, 0);
}

void InitImguiWindows()
{
	ImGui::Begin("Test");
	ImGui::End();

	ImGui::Begin("Another Window");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	ImGui::Text("Hello from another window!");
	ImGui::Button("Close Me");
}