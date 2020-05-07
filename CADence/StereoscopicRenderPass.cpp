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

}

void StereoscopicRenderPass::Execute(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
	auto context = renderState->m_device.m_context.get();
	auto depthStencil = renderState->m_depthBuffer.get();

	Clear(renderState);

	renderState->m_device.context()->VSSetShader(renderState->m_vertexShader.get(), nullptr, 0);
	renderState->m_device.context()->PSSetShader(renderState->m_pixelShader.get(), nullptr, 0);
	renderState->m_device.context()->IASetInputLayout(renderState->m_layout.get());

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
