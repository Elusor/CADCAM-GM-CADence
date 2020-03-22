#pragma once
#include "dxDevice.h"
#include "window.h"

static class GlobalRenderState
{
public:
	static DxDevice m_device;

	static void InitializeRenderState(mini::Window& window);
	static mini::dx_ptr<ID3D11InputLayout> CreateInputLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> elements);
	static void Clear();	
	static void SetShaders();
	static void RenderImGUI();

	template<class T>
	static mini::dx_ptr<ID3D11Buffer> CreateConstantBuffer();

private:
	static mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
	static mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
	static mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
	static mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
	static std::vector<BYTE> m_vsBytes;
	static std::vector<BYTE> m_psBytes;
	static float m_clearColor[4];

};