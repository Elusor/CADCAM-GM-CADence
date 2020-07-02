#include "CurveVisualizer.h"

CurveVisualizer::CurveVisualizer(GuiManager* manager, ID3D11Device* device, int width, int height)
{
	// Initialize the textures (1 for each surface)
	auto result = InitializeTextures(device, width, height);
	guiManager = manager;
	m_width = width;
	m_height = height;
	assert(result == true);
}

CurveVisualizer::~CurveVisualizer()
{
	if (m_shaderResourceView1)
	{
		m_shaderResourceView1->Release();
		m_shaderResourceView1 = 0;
	}

	if (m_renderTargetView1)
	{
		m_renderTargetView1->Release();
		m_renderTargetView1 = 0;
	}

	if (m_renderTargetTexture1)
	{
		m_renderTargetTexture1->Release();
		m_renderTargetTexture1 = 0;
	}

	if (m_shaderResourceView2)
	{
		m_shaderResourceView2->Release();
		m_shaderResourceView2 = 0;
	}

	if (m_renderTargetView2)
	{
		m_renderTargetView2->Release();
		m_renderTargetView2 = 0;
	}

	if (m_renderTargetTexture2)
	{
		m_renderTargetTexture2->Release();
		m_renderTargetTexture2 = 0;
	}

	return;
}

void CurveVisualizer::VisualizeCurve(IntersectionCurve* curve, std::unique_ptr<RenderState>& renderState)
{
	
	// Get Parameter lists from the curve
	auto params1 = curve->GetParameterList(IntersectedSurface::SurfaceP);
	auto params2 = curve->GetParameterList(IntersectedSurface::SurfaceQ);

	// Render the image onto the member texture
	RenderImage(renderState, m_renderTargetView1, params1);
	RenderImage(renderState, m_renderTargetView2, params2);

	// Call a new Imgui Window with texture section
	guiManager->EnableTextureWindow("", "Intersection curve", m_shaderResourceView1, m_width, m_height);
	guiManager->EnableTextureWindow("", "Intersection curve", m_shaderResourceView2, m_width, m_height);
}

ID3D11ShaderResourceView* CurveVisualizer::GetShaderResourceView(IntersectedSurface affectedSurface)
{
	ID3D11ShaderResourceView* srv;
	
	if (affectedSurface == IntersectedSurface::SurfaceQ)
	{
		srv = m_shaderResourceView1;
	}
	else {
		srv = m_shaderResourceView2;
	}

	return srv;
}

ID3D11RenderTargetView* CurveVisualizer::GetRenderTargetView(IntersectedSurface affectedSurface)
{
	ID3D11RenderTargetView* rtv;

	if (affectedSurface == IntersectedSurface::SurfaceQ)
	{
		rtv = m_renderTargetView1;
	}
	else {
		rtv = m_renderTargetView2;
	}

	return rtv;
}

void CurveVisualizer::RenderImage(std::unique_ptr<RenderState>& renderState, ID3D11RenderTargetView* texture, std::vector<DirectX::XMFLOAT2> paramList)
{
	// Clear the previous texture
	ClearTexture(texture, renderState->m_device.context().get(), renderState->m_depthBuffer.get(), 1.f, 1.f, 1.f, 1.f);
}

void CurveVisualizer::ClearTexture(ID3D11RenderTargetView* texture,
	ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencil,
	float r, float g, float b, float alpha, float depth)
{
	float color[4] = { r, g, b, alpha };
	context->ClearRenderTargetView(texture, color);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0.f);
}

bool CurveVisualizer::InitializeTextures(ID3D11Device* device, int width, int height)
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
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1; // Implement multisampling later
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&textureDesc, nullptr, &m_renderTargetTexture1);
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreateTexture2D(&textureDesc, nullptr, &m_renderTargetTexture2);
	if (FAILED(result))
	{
		return false;
	}

	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(m_renderTargetTexture1, &renderTargetViewDesc, &m_renderTargetView1);	
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreateRenderTargetView(m_renderTargetTexture2, &renderTargetViewDesc, &m_renderTargetView2);
	if (FAILED(result))
	{
		return false;
	}

	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(m_renderTargetTexture1, &shaderResourceViewDesc, &m_shaderResourceView1);
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreateShaderResourceView(m_renderTargetTexture2, &shaderResourceViewDesc, &m_shaderResourceView2);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}
