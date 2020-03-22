#include "Renderer.h"
#include "geometryRep.h"
#include "windows.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "RenderState.h"
#include "GlobalRenderState.h"
using namespace mini;
using namespace DirectX;

Renderer::Renderer(mini::Window& window)
{
	m_renderState->m_layout = GlobalRenderState::CreateInputLayout(VertexPositionColor::GetInputLayoutElements());	
	// Create constant buffer - MVP matrix
	m_renderState->m_cbMVP = GlobalRenderState::CreateConstantBuffer<XMFLOAT4X4>();
}

void Renderer::SetShaders()
{
	// TODO FIGURE OUT A CLASS FOR SHADERS AND ALSO CHANGE INPUT LAYOT WHEN SETTING SHADERS
	// Lighting/display style dependant (a little bit object dependant)
	// m_renderState->m_device.context()->VSSetShader(m_renderState->m_vertexShader.get(), nullptr, 0);
	// m_renderState->m_device.context()->PSSetShader(m_renderState->m_pixelShader.get(), nullptr, 0);
}

void Renderer::UpdateInputLayout()
{
	GlobalRenderState::m_device.context()->IASetInputLayout(m_renderState->m_layout.get());
}
