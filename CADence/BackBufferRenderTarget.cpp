#include "BackBufferRenderTarget.h"



BackBufferRenderTarget::BackBufferRenderTarget()
{
	m_renderTargetTexture = 0;
	m_renderTargetView = 0;
}

BackBufferRenderTarget::BackBufferRenderTarget(const BackBufferRenderTarget&)
{
}

BackBufferRenderTarget::~BackBufferRenderTarget()
{
}

bool BackBufferRenderTarget::Initialize(ID3D11Device* device, IDXGISwapChain* swapchain, RenderState* m_renderdata)
{
	//HRESULT result;	
	//ID3D11Texture2D* temp;
	//swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(temp));	
	//m_renderTargetTexture = temp;
	//// Create render target view to be able to write on backBuffer
	ID3D11Texture2D* temp;
	HRESULT result = m_renderdata->m_device.swapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&temp));
	if (FAILED(result))
	{
		return false;
	}

	mini::dx_ptr<ID3D11Texture2D> backTexture;
	backTexture.reset(temp);
	m_renderTargetTexture = backTexture.get();
	result = device->CreateRenderTargetView(m_renderTargetTexture, nullptr, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void BackBufferRenderTarget::Dispose()
{
	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if (m_renderTargetTexture)
	{
		m_renderTargetTexture->Release();
		m_renderTargetTexture = 0;
	}
}

void BackBufferRenderTarget::SetRenderTarget(ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencilView)
{
	// Bind the texture as the render pipeline output
	context->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);
}

void BackBufferRenderTarget::ClearRenderTarget(ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencilView, float r, float g, float b, float alpha)
{
	float color[4] = { r, g, b, alpha };

	context->ClearRenderTargetView(m_renderTargetView, color);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0.f);
}

ID3D11RenderTargetView* BackBufferRenderTarget::GetRenderTargetView()
{
	return m_renderTargetView;
}