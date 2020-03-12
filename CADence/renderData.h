#pragma once
#include "dxDevice.h"
#include "camera.h"
#include "window.h"
// Used to pass render data to objects so they can be drawn independently
struct RenderData
{
	RenderData(mini::Window& window);
	/*RenderData(DxDevice* device, Camera* camera, ID3D11Buffer* vBuffer, ID3D11Buffer* iBuffer, ID3D11Buffer* mvpBuffe);
	DxDevice* m_device;
	Camera* m_camera;
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	ID3D11Buffer* m_cbMVP;*/

	DxDevice m_device;
	mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
	mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
	mini::dx_ptr<ID3D11Buffer> m_vertexBuffer;
	mini::dx_ptr<ID3D11Buffer> m_indexBuffer;
	mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
	mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
	mini::dx_ptr<ID3D11InputLayout> m_layout;		

	DirectX::XMFLOAT4X4 m_modelMat, m_viewMat, m_projMat;
	mini::dx_ptr<ID3D11Buffer> m_cbMVP;
};
