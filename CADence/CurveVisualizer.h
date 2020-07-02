#pragma once
#include "IntersectionCurve.h"
#include "GuiManager.h"
class CurveVisualizer
{
public:
	CurveVisualizer(GuiManager* guiManager, ID3D11Device* device, int width, int height);
	~CurveVisualizer();
	void VisualizeCurve(IntersectionCurve* curve, std::unique_ptr<RenderState>& renderState);

	ID3D11ShaderResourceView* GetShaderResourceView(IntersectedSurface affectedSurface);
	ID3D11RenderTargetView* GetRenderTargetView(IntersectedSurface affectedSurface);
private:	
	int m_width, m_height;
	GuiManager* guiManager;
	DxDevice* device;
	ID3D11Texture2D* m_renderTargetTexture1;
	ID3D11Texture2D* m_renderTargetTexture2;
	ID3D11RenderTargetView* m_renderTargetView1;
	ID3D11RenderTargetView* m_renderTargetView2;
	ID3D11ShaderResourceView* m_shaderResourceView1;
	ID3D11ShaderResourceView* m_shaderResourceView2;

	void RenderImage(std::unique_ptr<RenderState>& renderState, ID3D11RenderTargetView* texture, std::vector<DirectX::XMFLOAT2> paramList);
	void ClearTexture(ID3D11RenderTargetView* texture,
		ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencil, 
		float r, float g, float b, float alpha, float depth = 1.0f);
	bool InitializeTextures(ID3D11Device* device, int width, int height);

};
