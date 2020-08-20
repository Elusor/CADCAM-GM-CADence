#include "DefaultRenderPass.h"

DefaultRenderPass::DefaultRenderPass(const std::unique_ptr<RenderState>& renderState, SIZE wndSize)
{
	renderState->m_depthBuffer = renderState->m_device.CreateDepthStencilView(wndSize);
	m_renderTarget = new BackBufferRenderTarget();
	m_renderTarget->Initialize(renderState->m_device.m_device.get(), renderState->m_device.m_swapChain.get(), renderState.get());	

	D3D11_RENDER_TARGET_BLEND_DESC rtBsDesc;
	ZeroMemory(&rtBsDesc, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
	rtBsDesc.BlendEnable = true;
	rtBsDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtBsDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtBsDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtBsDesc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	rtBsDesc.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	rtBsDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtBsDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D11_BLEND_DESC bsDesc;
	bsDesc.AlphaToCoverageEnable = false;
	bsDesc.IndependentBlendEnable = false;
	bsDesc.RenderTarget[0] = rtBsDesc;

	ID3D11BlendState* bs;
	auto HR = renderState->m_device->CreateBlendState(&bsDesc, &bs);
	m_blendState = dx_ptr<ID3D11BlendState>(bs);
}

void DefaultRenderPass::Execute(std::unique_ptr<RenderState>& renderState, Scene* scene)
{	
	Clear(renderState);
	

	FLOAT facs[4] = { 1.0f,1.0f,1.0f,1.0f };
	renderState->m_device.context()->OMSetBlendState(m_blendState.get(), facs, 0xffffffff);
	
	//Update Cam buffer
	XMFLOAT4 camPos = renderState->m_camera->GetCameraPosition();
	renderState->SetConstantBuffer<XMFLOAT4>(renderState->m_cbCamPos.get(), camPos);
	ID3D11Buffer* psCbs[] = { renderState->m_cbCamPos.get() };
	renderState->m_device.context()->PSSetConstantBuffers(0, 1, psCbs);
	//Update Fog buffer
	float nearZ = renderState->m_camera->GetZNear();
	float farZ = renderState->m_camera->GetZFar();
	XMFLOAT4 fogBuff = XMFLOAT4(nearZ, farZ, 0.8f * farZ, 0.0f);
	renderState->SetConstantBuffer<XMFLOAT4>(renderState->m_cbFogBuffer.get(), fogBuff);
	ID3D11Buffer* psFogCbs[] = { renderState->m_cbFogBuffer.get() };
	renderState->m_device.context()->PSSetConstantBuffers(1, 1, psFogCbs);


	m_renderTarget->SetRenderTarget(renderState->m_device.m_context.get(), renderState->m_depthBuffer.get());
	// Update viewprojection matrix
	XMMATRIX vp = renderState->m_camera->GetViewProjectionMatrix();
	auto VPbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbVP.get(), vp);
	ID3D11Buffer* cbs2[] = { VPbuffer };
	renderState->m_device.context()->VSSetConstantBuffers(0, 1, cbs2);

	Render(renderState, scene);
	//auto HR = renderState->m_device->CreateBlendState(&bsDesc, &bs);
	renderState->m_device.context()->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void DefaultRenderPass::Clear(std::unique_ptr<RenderState>& renderState)
{
	// Clear render target
	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_renderTarget->ClearRenderTarget(renderState->m_device.m_context.get(), renderState->m_depthBuffer.get(), 0.2f, 0.2f, 0.2f, 1.0f, 1.0f);
	//renderState->m_device.context()->ClearRenderTargetView(renderState->m_backBuffer.get(), clearColor);

	// Clera depth stencil
	renderState->m_device.context()->ClearDepthStencilView(renderState->m_depthBuffer.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
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