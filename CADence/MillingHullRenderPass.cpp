#include "MillingHullRenderPass.h"
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

MillingHullRenderPass::MillingHullRenderPass(std::unique_ptr<RenderState>& renderState, SIZE wndSize)
{
	auto device = renderState->m_device.m_device.get();

	float aspectRatio = (float)wndSize.cx / (float)wndSize.cy;
	float height = 50.f;
	m_camera = std::make_unique<OrthographicCamera>(
		15.f, 15.f, 
		1.0f, 50.f,
		Vector3(0.0f, 0.0f, -10.0f),
		Vector3(0.0f, 0.0f, 0.0f));
	m_texture = std::make_unique<TextureRenderTarget>();

	UINT resolution = 1000;

	m_texture->Initialize(
		device, 
		resolution, resolution);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Height = resolution;
	viewport.Width = resolution;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	m_viewPort = viewport;

}

void MillingHullRenderPass::Execute(std::unique_ptr<RenderState>& renderState, Scene* scene)
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
	renderState->currentCamera = m_camera.get();
	Clear(renderState);

	auto vpMat = m_camera->GetViewProjectionMatrix();
	auto VPbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbVP.get(), vpMat);
	ID3D11Buffer* cbs2[] = { VPbuffer };
	renderState->m_device.context()->VSSetConstantBuffers(0, 1, cbs2);

	Render(renderState, scene);

	// Restore old settings
	renderState->currentCamera = oldCamera;
	context->RSSetViewports(originalVPCount, originalVPs);
}

void MillingHullRenderPass::Clear(std::unique_ptr<RenderState>& renderState)
{
	// Clear render target
	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_texture->ClearRenderTarget(renderState->m_device.m_context.get(), renderState->m_depthBuffer.get(), 0.2f, 0.2f, 0.2f, 1.0f, 1.0f);

	// Clera depth stencil
	renderState->m_device.context()->ClearDepthStencilView(renderState->m_depthBuffer.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void MillingHullRenderPass::Render(std::unique_ptr<RenderState>& renderState, Scene* scene)
{
	auto depthStencil = renderState->m_depthBuffer.get();
	auto context = renderState->m_device.m_context.get();

	m_texture->SetRenderTarget(context, depthStencil);
	renderState->m_device.context()->VSSetShader(renderState->m_vertexShader.get(), nullptr, 0);
	renderState->m_device.context()->PSSetShader(renderState->m_pixelShader.get(), nullptr, 0);

	renderState->m_device.context()->IASetInputLayout(renderState->GetLayout(std::type_index(typeid(VertexPositionColor))));

	// Render Actual scene
	scene->RenderScene(renderState);
}
