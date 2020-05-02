#pragma once
#include <d3d11.h>
#include "renderState.h"
#include "IRenderTarget.h"

class BackBufferRenderTarget: public IRenderTarget
{
public:
	BackBufferRenderTarget();
	BackBufferRenderTarget(const BackBufferRenderTarget&);
	~BackBufferRenderTarget();

	virtual bool Initialize(ID3D11Device* device, IDXGISwapChain* swapchain, RenderState* m_renderState);
	
	// Inherited via IRenderTarget
	virtual void Dispose() override;
	virtual void SetRenderTarget(
		ID3D11DeviceContext* context,
		ID3D11DepthStencilView* depthStencilView) override;
	virtual void ClearRenderTarget(
		ID3D11DeviceContext* context,
		ID3D11DepthStencilView* depthStencilView,
		float r, float g, float b, float alpha, float depth) override;

	ID3D11RenderTargetView* GetRenderTargetView();

private:
	ID3D11Texture2D* m_renderTargetTexture;
	ID3D11RenderTargetView* m_renderTargetView;	
};