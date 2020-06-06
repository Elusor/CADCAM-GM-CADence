#include "StereoscopicRenderPass.h"

StereoscopicRenderPass::StereoscopicRenderPass(const std::unique_ptr<RenderState>& renderState, SIZE wndSize)
{
	auto device = renderState->m_device.m_device.get();
	auto context = renderState->m_device.m_context.get();

	m_tex1 = new TextureRenderTarget();
	m_tex2 = new TextureRenderTarget();
	m_backTarget = new BackBufferRenderTarget();
	// Load proper shaders
	const auto texVsBytes = DxDevice::LoadByteCode(L"vsTex.cso");
	const auto texPsBytes = DxDevice::LoadByteCode(L"psTex.cso");

	m_texVS = renderState->m_device.CreateVertexShader(texVsBytes);
	m_texPS = renderState->m_device.CreatePixelShader(texPsBytes);

	m_tex1->Initialize(device, wndSize.cx, wndSize.cy);
	m_tex2->Initialize(device, wndSize.cx, wndSize.cy);
	m_backTarget->Initialize(device, renderState->m_device.m_swapChain.get(), renderState.get());

	D3D11_SAMPLER_DESC desc;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.MinLOD = 0;
	desc.MaxLOD = 0;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.BorderColor[0] = 1.0f;
	desc.BorderColor[1] = 1.0f;
	desc.BorderColor[2] = 1.0f;
	desc.BorderColor[3] = 1.0f;


	ID3D11SamplerState* ss;
	m_sampler = mini::dx_ptr<ID3D11SamplerState>();
	device->CreateSamplerState(&desc, &ss);	
	m_sampler.reset(ss);

	m_quadVerts = {
		{ {-1.0f,-1.0f,0.0f}, {1.0f,1.0f,1.0f} },
		{ {-1.0f, 1.0f,0.0f}, {1.0f,1.0f,1.0f} },
		{ { 1.0f, 1.0f,0.0f}, {1.0f,1.0f,1.0f} },
		{ { 1.0f,-1.0f,0.0f}, {1.0f,1.0f,1.0f} }		
	};

	m_quadIndices = {
		0, 1, 3,
		3, 1, 2
	};

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
	m_blendState = mini::dx_ptr<ID3D11BlendState>(bs);

}

void StereoscopicRenderPass::Execute(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
	auto context = renderState->m_device.m_context.get();
	auto depthStencil = renderState->m_depthBuffer.get();

	Clear(renderState);
	FLOAT facs[4] = { 1.0f,1.0f,1.0f,1.0f };
	renderState->m_device.context()->OMSetBlendState(m_blendState.get(), facs, 0xffffffff);
	renderState->m_device.context()->VSSetShader(renderState->m_vertexShader.get(), nullptr, 0);
	renderState->m_device.context()->PSSetShader(renderState->m_pixelShader.get(), nullptr, 0);
	renderState->m_device.context()->IASetInputLayout(renderState->m_layout.get());

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

	// Draw Left eye
	// Update viewprojection matrix
	XMMATRIX vp = renderState->m_camera->GetStereoscopicMatrix(true, m_eyeDistance, m_focusPlaneDistance);
	auto VPbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbVP.get(), vp);
	ID3D11Buffer* cbs2[] = { VPbuffer };
	context->VSSetConstantBuffers(0, 1, cbs2);

	m_tex1->SetRenderTarget(context, depthStencil);
	scene->RenderScene(renderState);	
	ClearDepth(renderState);	

	// Draw Right eye
	// Update viewprojection matrix
	vp = renderState->m_camera->GetStereoscopicMatrix(false, m_eyeDistance, m_focusPlaneDistance);
	auto VPbuffer2 = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbVP.get(), vp);
	ID3D11Buffer* cbs3[] = { VPbuffer2 };
	context->VSSetConstantBuffers(0, 1, cbs3);

	m_tex2->SetRenderTarget(context, depthStencil);
	scene->RenderScene(renderState);

	ClearDepth(renderState);	
	
	DrawTexturedQuad(renderState);
	//renderState->m_device.context()->OMSetBlendState(nullptr, nullptr, 0xffffffff);

}

void StereoscopicRenderPass::Clear(std::unique_ptr<RenderState>& renderState)
{
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	auto context = renderState->m_device.m_context.get();
	auto depthStencil = renderState->m_depthBuffer.get();
	m_tex1->ClearRenderTarget(context, depthStencil, clearColor[0], clearColor[1], clearColor[2], clearColor[3], 1.0f);
	m_tex2->ClearRenderTarget(context, depthStencil, clearColor[0], clearColor[1], clearColor[2], clearColor[3], 1.0f);
	m_backTarget->ClearRenderTarget(context, depthStencil, clearColor[0], clearColor[1], clearColor[2], clearColor[3], 1.0f);
}

void StereoscopicRenderPass::Render(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
}

void StereoscopicRenderPass::ClearDepth(std::unique_ptr<RenderState>& renderState)
{
	auto depthStencil = renderState->m_depthBuffer.get();
	renderState->m_device.context()->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void StereoscopicRenderPass::DrawTexturedQuad(std::unique_ptr<RenderState>& renderState)
{
	auto device = renderState->m_device.m_device.get();
	auto context = renderState->m_device.m_context.get();
	auto depthStencil = renderState->m_depthBuffer.get();

	m_backTarget->SetRenderTarget(context, depthStencil);
	// set texture blending shaders
	context->VSSetShader(m_texVS.get(), nullptr, 0);
	context->PSSetShader(m_texPS.get(), nullptr, 0);
	//renderState->m_device.context()->IASetInputLayout(renderState->m_layout.get());

	auto vp = renderState->m_camera->GetViewProjectionMatrix();
	auto VPbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbVP.get(), vp);
	ID3D11Buffer* cbs4[] = { VPbuffer };
	context->VSSetConstantBuffers(0, 1, cbs4);

	// Set shader resources
	ID3D11ShaderResourceView* tex1 = m_tex1->GetShaderResourceView();
	ID3D11ShaderResourceView* tex2 = m_tex2->GetShaderResourceView();
	ID3D11ShaderResourceView* res[] = { tex1, tex2 };
	context->PSSetShaderResources(0, 2, res);
	ID3D11SamplerState* states[] = { m_sampler.get() };
	context->PSSetSamplers(0, 1, states);

	// set an object taking up the whole screen and texture is 
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	renderState->m_vertexBuffer = (renderState->m_device.CreateVertexBuffer(m_quadVerts));
	renderState->m_indexBuffer = (renderState->m_device.CreateIndexBuffer(m_quadIndices));
	ID3D11Buffer* vbs[] = { renderState->m_vertexBuffer.get() };
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };
	context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	context->IASetIndexBuffer(renderState->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);

	// draw the textured object	
	context->DrawIndexed(m_quadIndices.size(), 0, 0);
}
