#include "DefaultRenderPass.h"

DefaultRenderPass::DefaultRenderPass(const std::unique_ptr<RenderState>& renderState, SIZE wndSize)
{
	renderState->m_depthBuffer = renderState->m_device.CreateDepthStencilView(wndSize);
	m_renderTarget = new BackBufferRenderTarget();
	m_renderTarget->Initialize(renderState->m_device.m_device.get(), renderState->m_device.m_swapChain.get(), renderState.get());
	m_renderTarget->SetRenderTarget(renderState->m_device.m_context.get(), renderState->m_depthBuffer.get());
}

void DefaultRenderPass::Execute(std::unique_ptr<RenderState>& renderState, Scene* scene)
{	
	Clear(renderState);

	// Update viewprojection matrix
	XMMATRIX vp = renderState->m_camera->GetViewProjectionMatrix();
	auto VPbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbVP.get(), vp);
	ID3D11Buffer* cbs2[] = { VPbuffer };
	renderState->m_device.context()->VSSetConstantBuffers(0, 1, cbs2);

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
	// Set default shaders
	renderState->m_device.context()->VSSetShader(renderState->m_vertexShader.get(), nullptr, 0);
	renderState->m_device.context()->PSSetShader(renderState->m_pixelShader.get(), nullptr, 0);

	renderState->m_device.context()->IASetInputLayout(renderState->m_layout.get());

	// Render Actual scene
	scene->RenderScene(renderState);
}