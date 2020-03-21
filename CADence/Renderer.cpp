#include "Renderer.h"
#include "geometryRep.h"
#include "windows.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

using namespace mini;
using namespace DirectX;

Renderer::Renderer(mini::Window& window)
{
	// Create RenderState obj
	m_renderState = unique_ptr<RenderState>(new RenderState(window));
	SIZE wndSize = window.getClientSize();

	// Create and set viewport
	Viewport viewport{ wndSize };
	m_renderState->m_device.context()->RSSetViewports(1, &viewport);

	// init camera
	m_renderState->m_camera = std::shared_ptr<Camera>(
		new Camera(
			DirectX::XMFLOAT3(0.0f, 0.0f, -30.0f), // camera pos 
			DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),  // targer pos 
			DirectX::XMFLOAT2(0.0f, 0.0f), // yaw, pitch
			viewport.Width,
			viewport.Height,
			45.0f, 2.5f, 250.0f)); // fov, zNear, zFar


	// init backbuffer
	ID3D11Texture2D* temp;
	m_renderState->m_device.swapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&temp));
	dx_ptr<ID3D11Texture2D> backTexture;
	backTexture.reset(temp);

	// Create render target view to be able to write on backBuffer
	m_renderState->m_backBuffer = m_renderState->m_device.CreateRenderTargetView(backTexture);

	// assign depth buffer to RP
	m_renderState->m_depthBuffer = m_renderState->m_device.CreateDepthStencilView(wndSize);

	// set render target
	auto backBuffer = m_renderState->m_backBuffer.get();
	m_renderState->m_device.context()->OMSetRenderTargets(1, &backBuffer, m_renderState->m_depthBuffer.get());

	const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
	const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");

	m_renderState->m_vertexShader = m_renderState->m_device.CreateVertexShader(vsBytes);
	m_renderState->m_pixelShader = m_renderState->m_device.CreatePixelShader(psBytes);

	// Create input layout
	auto elements = VertexPositionColor::GetInputLayoutElements();
	m_renderState->m_layout = m_renderState->m_device.CreateInputLayout(elements, vsBytes);

	// Create constant buffer - MVP matrix
	m_renderState->m_cbMVP = m_renderState->m_device.CreateConstantBuffer<XMFLOAT4X4>();	

	// Init Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(window.getHandle());
	ImGui_ImplDX11_Init(m_renderState->m_device.m_device.get(), m_renderState->m_device.m_context.get());
	ImGui::StyleColorsDark();
}

void Renderer::Clear()
{	
	// Clear back buffer
	m_renderState->m_device.context()->ClearRenderTargetView(m_renderState->m_backBuffer.get(), m_clearColor);

	// Clera depth stencil
	m_renderState->m_device.context()->ClearDepthStencilView(m_renderState->m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}


void Renderer::Render(std::unique_ptr<Transform>& transform)
{
}

void Renderer::RenderImGUI()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	m_renderState->m_device.m_swapChain.get()->Present(0, 0);
}

void Renderer::SetShaders()
{
	// Lighting/display style dependant (a little bit object dependant)
	m_renderState->m_device.context()->VSSetShader(m_renderState->m_vertexShader.get(), nullptr, 0);
	m_renderState->m_device.context()->PSSetShader(m_renderState->m_pixelShader.get(), nullptr, 0);
}

void Renderer::UpdateInputLayout()
{
	m_renderState->m_device.context()->IASetInputLayout(m_renderState->m_layout.get());
}
