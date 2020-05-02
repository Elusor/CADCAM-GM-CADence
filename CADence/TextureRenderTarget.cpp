#include "TextureRenderTarget.h"

TextureRenderTarget::TextureRenderTarget()
{
	m_renderTargetTexture = 0;
	m_renderTargetView = 0;
	m_shaderResourceView = 0;
}

TextureRenderTarget::TextureRenderTarget(const TextureRenderTarget&)
{
}

TextureRenderTarget::~TextureRenderTarget()
{
}

bool TextureRenderTarget::Initialize(ID3D11Device* device, int width, int height)
{
	// initialize the texture 

	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	// Set up texture description
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	textureDesc.SampleDesc.Count = 1; // Implement multisampling later
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&textureDesc, nullptr, &m_renderTargetTexture);
	if (FAILED(result))
	{
		return false;
	}

	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(m_renderTargetTexture, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void TextureRenderTarget::Dispose()
{
	if (m_shaderResourceView)
	{
		m_shaderResourceView->Release();
		m_shaderResourceView = 0;
	}

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

	return;
}

void TextureRenderTarget::SetRenderTarget(ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencilView)
{	
	// Bind the texture as the render pipeline output
	context->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);
}

void TextureRenderTarget::ClearRenderTarget(ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencil, float r, float g, float b, float alpha, float depth = 1.0f)
{
	float color[4] = {r, g, b, alpha};

	context->ClearRenderTargetView(m_renderTargetView, color);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0.f);
}

ID3D11ShaderResourceView* TextureRenderTarget::GetShaderResourceView()
{
	return m_shaderResourceView;
}

ID3D11RenderTargetView* TextureRenderTarget::GetRenderTargetView()
{
	return m_renderTargetView;
}