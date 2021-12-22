#include "MillingHullRenderPass.h"
#include "SimpleMath.h"
#include "dxDevice.h"
using namespace DirectX::SimpleMath;

MillingHullRenderPass::MillingHullRenderPass(
	std::unique_ptr<RenderState>& renderState, 
	float cameraSide,
	float zNear,
	float zFar, 
	UINT resolution)
{
	auto& dxDevice = renderState->m_device;
	auto device = renderState->m_device.m_device.get();
	
	m_camera = std::make_unique<OrthographicCamera>(
		cameraSide, cameraSide,
		zNear, zFar,
		Vector3(0.0f, 0.0f, -zFar),
		Vector3(0.0f, 0.0f, 0.0f));

	m_cameraCloser = std::make_unique<OrthographicCamera>(
		cameraSide, cameraSide,
		zNear, zFar,
		Vector3(0.0f, 0.0f, -zFar + 1.2f),
		Vector3(0.0f, 0.0f, 0.0f));
	m_texture = std::make_unique<TextureRenderTarget>();

	m_texture->Initialize(
		device, 
		resolution, resolution);

	CreateViewport(resolution);
	CreateDepthStencil(renderState, resolution, zNear, zFar);
}

void MillingHullRenderPass::Execute(std::unique_ptr<RenderState>& renderState, PathModel* model)
{
	auto context = renderState->m_device.m_context.get();

	// Store viewport
	UINT originalVPCount = 1;
	// Probe the current viewport count
	context->RSGetViewports(&originalVPCount, NULL);
	// Get current viewport
	D3D11_VIEWPORT* originalVPs = new D3D11_VIEWPORT[originalVPCount];
	context->RSGetViewports(&originalVPCount, originalVPs);

	// Set visualizer viewport
	context->RSSetViewports(1, &m_viewPort);
	// Set new viewport

	auto oldCamera = renderState->currentCamera;
	renderState->currentCamera = m_cameraCloser.get();
	Clear(renderState);

	auto vpMatCloser = m_cameraCloser->GetViewProjectionMatrix();
	auto VPbufferCloser = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbVP.get(), vpMatCloser);
	ID3D11Buffer* cbs2[] = { VPbufferCloser };
	renderState->m_device.context()->VSSetConstantBuffers(0, 1, cbs2);
	
	XMFLOAT4 offset16byteAligned = XMFLOAT4(0.0f, 0.f, 0.f, 0.f);
	auto offsetBuff = renderState->SetConstantBuffer<XMFLOAT4>(renderState->m_cbMillingOffset.get(), offset16byteAligned);
	renderState->m_device.context()->GSSetConstantBuffers(6, 1, &offsetBuff);

	Render(renderState, model, true);

	renderState->currentCamera = m_camera.get();
	auto vpMat = m_camera->GetViewProjectionMatrix();
	auto VPbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbVP.get(), vpMat);
	ID3D11Buffer* cbs3[] = { VPbuffer };
	renderState->m_device.context()->VSSetConstantBuffers(0, 1, cbs3);

	offset16byteAligned = XMFLOAT4(m_offset, 0.f, 0.f, 0.f);
	offsetBuff = renderState->SetConstantBuffer<XMFLOAT4>(renderState->m_cbMillingOffset.get(), offset16byteAligned);
	renderState->m_device.context()->GSSetConstantBuffers(6, 1, &offsetBuff);
	Render(renderState, model);

	// Restore old settings
	renderState->currentCamera = oldCamera;
	context->RSSetViewports(originalVPCount, originalVPs);
}

void MillingHullRenderPass::Execute(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
}

void MillingHullRenderPass::Clear(std::unique_ptr<RenderState>& renderState)
{
	// Clear render target
	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_texture->ClearRenderTarget(renderState->m_device.m_context.get(), m_dsv.get(), 0.2f, 0.2f, 0.2f, 1.0f, 1.0f);

	// Clera depth stencil
	renderState->m_device.context()->ClearDepthStencilView(m_dsv.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void MillingHullRenderPass::Render(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
}

void MillingHullRenderPass::Render(std::unique_ptr<RenderState>& renderState, PathModel* model, bool baseOnly)
{
	auto context = renderState->m_device.m_context.get();

	m_texture->SetRenderTarget(context, m_dsv.get());
	renderState->m_device.context()->VSSetShader(renderState->m_vertexShader.get(), nullptr, 0);
	renderState->m_device.context()->PSSetShader(renderState->m_pixelShader.get(), nullptr, 0);

	renderState->m_device.context()->IASetInputLayout(renderState->GetLayout(std::type_index(typeid(VertexPositionColor))));

	auto allObjects = model->GetModelObjects();
	auto modelObjects = allObjects;
	modelObjects.erase(modelObjects.end()-1);

	if (baseOnly)
	{
		auto sideSpikes = allObjects[5];
		if (auto ptr = sideSpikes.lock())
		{			
			ptr->Render(renderState);
		}
	}
	else
	{
		// Render Actual scene
		for (auto wPtr : modelObjects)
		{
			if (auto ptr = wPtr.lock())
			{
				ptr->Render(renderState);
			}
		}
		auto base = allObjects[allObjects.size() - 1];
		if (auto ptr = base.lock())
		{
			ptr->Render(renderState);
		}
	}

	
}

void MillingHullRenderPass::SetOffset(float val)
{
	m_offset = val;
}

float MillingHullRenderPass::GetOffset()
{
	return m_offset;
}

ID3D11Texture2D* MillingHullRenderPass::GetDepthTex()
{
	return m_depthTex.get();
}

void MillingHullRenderPass::CreateDepthStencil(std::unique_ptr<RenderState>& renderState, UINT resolution, float minZ, float maxZ)
{

	auto& dxDevice = renderState->m_device;

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Format = DXGI_FORMAT_D32_FLOAT;
	texDesc.MipLevels = 1;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.Width = resolution;
	texDesc.Height = resolution;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	//CPUAccessFlags = 0;
	//MiscFlags = 0;

	m_depthTex = dxDevice.CreateTexture(texDesc);
	m_dsv = dxDevice.CreateDepthStencilView(m_depthTex);
}

void MillingHullRenderPass::CreateViewport(UINT resolution)
{
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Height = resolution;
	viewport.Width = resolution;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	m_viewPort = viewport;
}
