#pragma once
#include <d3d11.h>
#include "renderState.h"

class BackBufferRenderTarget
{
public:
	BackBufferRenderTarget();
	BackBufferRenderTarget(const BackBufferRenderTarget&);
	~BackBufferRenderTarget();

	bool Initialize(ID3D11Device* device, IDXGISwapChain* swapchain, RenderState* m_renderdata);
	void Dispose();

	void SetRenderTarget(ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencilView);
	void ClearRenderTarget(ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencilView, float r, float g, float b, float alpha);
	ID3D11RenderTargetView* GetRenderTargetView();

private:
	ID3D11Texture2D* m_renderTargetTexture;
	ID3D11RenderTargetView* m_renderTargetView;
};