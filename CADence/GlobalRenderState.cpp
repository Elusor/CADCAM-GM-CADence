#include "GlobalRenderState.h"
#include "imgui.h"
#include "geometryRep.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

void GlobalRenderState::InitializeRenderState(mini::Window& window)
{	
	GlobalRenderState::m_clearColor[0] = 0.2f;
	GlobalRenderState::m_clearColor[1] = 0.2f;
	GlobalRenderState::m_clearColor[2] = 0.2f;
	GlobalRenderState::m_clearColor[3] = 1.0f;

	m_device = DxDevice(window);
	SIZE wndSize = window.getClientSize();

	// Create and set viewport
	Viewport viewport{ wndSize };
	m_device.context()->RSSetViewports(1, &viewport);
	
	// init backbuffer
	ID3D11Texture2D* temp;
	m_device.swapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&temp));
	mini::dx_ptr<ID3D11Texture2D> backTexture;
	backTexture.reset(temp);

	// Create render target view to be able to write on backBuffer
	m_backBuffer = m_device.CreateRenderTargetView(backTexture);

	// assign depth buffer to RP
	m_depthBuffer = m_device.CreateDepthStencilView(wndSize);

	// set render target
	auto backBuffer = m_backBuffer.get();
	m_device.context()->OMSetRenderTargets(1, &backBuffer, m_depthBuffer.get());

	m_vsBytes = DxDevice::LoadByteCode(L"vs.cso");
	m_psBytes = DxDevice::LoadByteCode(L"ps.cso");

	m_vertexShader = m_device.CreateVertexShader(m_vsBytes);
	m_pixelShader = m_device.CreatePixelShader(m_psBytes);

	// Create input layout
	auto elements = VertexPositionColor::GetInputLayoutElements();

	// Create constant buffer - MVP matrix
	//m_cbMVP = m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>();

	// Init Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(window.getHandle());
	ImGui_ImplDX11_Init(m_device.m_device.get(), m_device.m_context.get());
	ImGui::StyleColorsDark();
	
	// TODO [MG] Move this somewhere else if shaders change
	SetShaders();

}

template<class T>
mini::dx_ptr<ID3D11Buffer> GlobalRenderState::CreateConstantBuffer()
{
	return m_device.CreateConstantBuffer<T>();
}

mini::dx_ptr<ID3D11InputLayout> GlobalRenderState::CreateInputLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> elements)
{
	return m_device.CreateInputLayout(elements, m_vsBytes);
}

void GlobalRenderState::SetShaders()
{
	m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
	m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
}

void GlobalRenderState::RenderImGUI()
{	
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	GlobalRenderState::m_device.m_swapChain.get()->Present(0, 0);	
}

// Clears render targer and depth stencil
void GlobalRenderState::Clear()
{
	// Clear back buffer
	m_device.context()->ClearRenderTargetView(m_backBuffer.get(), m_clearColor);

	// Clera depth stencil
	m_device.context()->ClearDepthStencilView(m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}