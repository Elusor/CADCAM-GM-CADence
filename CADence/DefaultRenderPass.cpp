#include "DefaultRenderPass.h"

void DefaultRenderPass::Execute(std::unique_ptr<RenderState>& renderState, Scene* scene)
{	
	Clear(renderState);
	Render(renderState, scene);
}

void DefaultRenderPass::Clear(std::unique_ptr<RenderState>& renderState)
{
	// Clear render target
	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_renderTarget->ClearRenderTarget(renderState->m_device.m_context.get(), renderState->m_depthBuffer.get(), 0.2f, 0.2f, 0.2f, 1.0f, 1.0f);
	renderState->m_device.context()->ClearRenderTargetView(renderState->m_backBuffer.get(), clearColor);

	// Clera depth stencil
	renderState->m_device.context()->ClearDepthStencilView(renderState->m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DefaultRenderPass::Render(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
	// Render Actual scene
	renderState->m_device.context()->VSSetShader(renderState->m_vertexShader.get(), nullptr, 0);
	renderState->m_device.context()->PSSetShader(renderState->m_pixelShader.get(), nullptr, 0);

	// object dependant
	renderState->m_device.context()->IASetInputLayout(renderState->m_layout.get());


	scene->RenderScene(renderState);
}