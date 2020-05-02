#include "StereoscopicRenderPass.h"

StereoscopicRenderPass::StereoscopicRenderPass(const std::unique_ptr<RenderState>& renderState, SIZE wndSize)
{
	m_tex1 = new TextureRenderTarget();
	m_tex2 = new TextureRenderTarget();
	m_backTarget = new BackBufferRenderTarget();
	m_tex1->Initialize(renderState->m_device.m_device.get(), wndSize.cx, wndSize.cy);
	m_tex2->Initialize(renderState->m_device.m_device.get(), wndSize.cx, wndSize.cy);
	m_backTarget->Initialize(renderState->m_device.m_device.get(), renderState->m_device.m_swapChain.get(), renderState.get());

}

void StereoscopicRenderPass::Execute(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
	auto context = renderState->m_device.m_context.get();
	auto depthStencil = renderState->m_depthBuffer.get();

	Clear(renderState);

#pragma region left eye	
	// Draw Left eye
	// Update viewprojection matrix
	XMMATRIX vp = renderState->m_camera->GetViewProjectionMatrix();
	auto VPbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbVP.get(), vp);
	ID3D11Buffer* cbs2[] = { VPbuffer };
	renderState->m_device.context()->VSSetConstantBuffers(0, 1, cbs2);

	m_tex1->SetRenderTarget(context, depthStencil);
	scene->RenderScene(renderState);	
#pragma endregion

	// TODO CLEAR DEPTH STENCIL VIEW

#pragma region right eye	
	// Draw Right eye
	// Update viewprojection matrix
	XMMATRIX vp = renderState->m_camera->GetViewProjectionMatrix();
	auto VPbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbVP.get(), vp);
	ID3D11Buffer* cbs2[] = { VPbuffer };
	renderState->m_device.context()->VSSetConstantBuffers(0, 1, cbs2);

	m_tex2->SetRenderTarget(context, depthStencil);
	scene->RenderScene(renderState);
#pragma endregion

	ID3D11ShaderResourceView* tex1 = m_tex1->GetShaderResourceView();
	ID3D11ShaderResourceView* tex2 = m_tex2->GetShaderResourceView();
	m_backTarget->SetRenderTarget(context, depthStencil);
	// set texture blending shaders

	// set tex1 and tex2 as shader resources

	// set an object taking up the whole screen and texture is 
	
	// draw the textured object	
	//renderState->m_device.m_context->DrawIndexed();
}

void StereoscopicRenderPass::Clear(std::unique_ptr<RenderState>& renderState)
{
	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	auto context = renderState->m_device.m_context.get();
	auto depthStencil = renderState->m_depthBuffer.get();
	m_tex1->ClearRenderTarget(context, depthStencil, clearColor[0], clearColor[1], clearColor[2], clearColor[3], 1.0f);
	m_tex2->ClearRenderTarget(context, depthStencil, clearColor[0], clearColor[1], clearColor[2], clearColor[3], 1.0f);
	m_backTarget->ClearRenderTarget(context, depthStencil, clearColor[0], clearColor[1], clearColor[2], clearColor[3], 1.0f);
}

void StereoscopicRenderPass::Render(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
}
