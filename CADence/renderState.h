#pragma once
#include "dxDevice.h"
#include "camera.h"
#include "window.h"
// Used to pass render data to objects so they can be drawn independently
struct RenderState
{
	RenderState(mini::Window& window);

	std::shared_ptr<Camera> m_camera;
	DxDevice m_device;
	mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
	mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
	mini::dx_ptr<ID3D11Buffer> m_vertexBuffer;
	mini::dx_ptr<ID3D11Buffer> m_indexBuffer;
	mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
	mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
	mini::dx_ptr<ID3D11GeometryShader> m_bezierGeometryShader;
	mini::dx_ptr<ID3D11InputLayout> m_layout;		

	DirectX::XMFLOAT4X4 m_modelMat, m_viewMat, m_projMat;
	mini::dx_ptr<ID3D11Buffer> m_cbMVP;
	mini::dx_ptr<ID3D11Buffer> m_cbGSData;
	
	template <typename T>
	ID3D11Buffer* SetConstantBuffer(ID3D11Buffer* buffer, T writeData);
};

template<typename T>
ID3D11Buffer* RenderState::SetConstantBuffer(ID3D11Buffer* buffer, T writeData)
{
	D3D11_MAPPED_SUBRESOURCE res;
	auto hres = m_device.context()->Map((buffer), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, &writeData, sizeof(T));
	m_device.context()->Unmap(buffer, 0);
	ID3D11Buffer* cbs = buffer;
	return cbs;
}

template ID3D11Buffer* RenderState::SetConstantBuffer<DirectX::XMMATRIX>(ID3D11Buffer* buffer, DirectX::XMMATRIX writeData);
template ID3D11Buffer* RenderState::SetConstantBuffer<DirectX::XMVECTOR>(ID3D11Buffer* buffer, DirectX::XMVECTOR writeData);
