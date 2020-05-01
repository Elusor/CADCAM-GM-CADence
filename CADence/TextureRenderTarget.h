#include <d3d11.h>
#pragma once
class TextureRenderTarget
{
public:
	TextureRenderTarget();
	TextureRenderTarget(const TextureRenderTarget&);
	~TextureRenderTarget();

	bool Initialize(ID3D11Device* device, int width, int height);
	void Dispose();

	void SetRenderTarget(ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencilView);
	void ClearRenderTarget(ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencilView, float r, float g, float b, float alpha);
	ID3D11ShaderResourceView* GetShaderResourceView();
	ID3D11RenderTargetView* GetRenderTargetView();
private:
	ID3D11Texture2D* m_renderTargetTexture;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11ShaderResourceView* m_shaderResourceView;
};