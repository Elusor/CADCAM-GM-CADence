#pragma once
#include "dxDevice.h"
#include "camera.h"
#include "window.h"
// Used to pass render data to objects so they can be drawn independently
struct RenderState
{
	mini::dx_ptr<ID3D11Buffer> m_vertexBuffer;
	mini::dx_ptr<ID3D11Buffer> m_indexBuffer;	
	mini::dx_ptr<ID3D11InputLayout> m_layout;		
	mini::dx_ptr<ID3D11Buffer> m_cbMVP;
	DirectX::XMFLOAT4X4 m_modelMat;
};
