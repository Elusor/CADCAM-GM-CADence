#pragma once
#include <DirectXMath.h>
#include <unordered_map>
#include <typeindex>
#include <stdexcept>

#include "dxDevice.h"
#include "camera.h"
#include "window.h"
#include "vertexStructures.h"
#include "InputLayoutManager.h"
using namespace DirectX;


struct ShaderPreset
{
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11DomainShader* domainShader = nullptr;
	ID3D11HullShader* hullShader = nullptr;
	ID3D11GeometryShader* geometryShader = nullptr;
};

// Used to pass render data to objects so they can be drawn independently
// TODO: rework this class 
class RenderState
{
public:
	InputLayoutManager m_layoutManager;
	DxDevice m_device;
	std::shared_ptr<Camera> m_camera;
	mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
	mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
	mini::dx_ptr<ID3D11Buffer> m_vertexBuffer;
	mini::dx_ptr<ID3D11Buffer> m_indexBuffer;

	mini::dx_ptr<ID3D11VertexShader> m_paramVS;

	mini::dx_ptr<ID3D11VertexShader> m_texVS;
	mini::dx_ptr<ID3D11PixelShader> m_texPS;

	mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
	mini::dx_ptr<ID3D11PixelShader> m_pixelShader;

	mini::dx_ptr<ID3D11VertexShader> m_screenSpaceVS;
	mini::dx_ptr<ID3D11PixelShader> m_screenSpacePS;

	mini::dx_ptr<ID3D11VertexShader> m_paramSpaceVS;
	mini::dx_ptr<ID3D11PixelShader> m_paramSpacePS;

	mini::dx_ptr<ID3D11GeometryShader> m_patchParamGeometryShader;
	mini::dx_ptr<ID3D11GeometryShader> m_patchC2ParamGeometryShader;
	mini::dx_ptr<ID3D11GeometryShader> m_torusGeometryShader;
	mini::dx_ptr<ID3D11GeometryShader> m_bezierGeometryShader;
	mini::dx_ptr<ID3D11GeometryShader> m_patchGeometryShader;
	mini::dx_ptr<ID3D11HullShader> m_patchHullShader;
	mini::dx_ptr<ID3D11HullShader> m_patchGregHullShader;
	mini::dx_ptr<ID3D11DomainShader> m_patchDomainShader;
	mini::dx_ptr<ID3D11DomainShader> m_patchC2DomainShader;
	mini::dx_ptr<ID3D11DomainShader> m_patchGregDomainShader;
	
	DirectX::XMFLOAT4X4 m_modelMat, m_viewMat, m_projMat;

	mini::dx_ptr<ID3D11Buffer> m_cbCamPos;
	mini::dx_ptr<ID3D11Buffer> m_cbFogBuffer;
	mini::dx_ptr<ID3D11Buffer> m_cbVP;
	mini::dx_ptr<ID3D11Buffer> m_cbM;
	mini::dx_ptr<ID3D11Buffer> m_cbGSData;
	mini::dx_ptr<ID3D11Buffer> m_cbTorusData;
	mini::dx_ptr<ID3D11Buffer> m_cbPatchData;
	mini::dx_ptr<ID3D11Buffer> m_cbPatchData1;
	mini::dx_ptr<ID3D11Buffer> m_cbPatchData2;
	mini::dx_ptr<ID3D11Buffer> m_cbPatchData3;

	mini::dx_ptr<ID3D11Buffer> m_cbPatchDivisions;
	
	RenderState(mini::Window& window);
	RenderState(mini::Window& window, Viewport vp, std::shared_ptr<Camera> camera);
	
	ID3D11InputLayout* GetLayout(std::type_index vertexDataTypeIndex);

	ShaderPreset GetCurrentShaderPreset();
	void SetShaderPreset(ShaderPreset preset);

	template <class T>
	void RegisterLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutElements, const std::vector<BYTE> vsCode);

	template <typename T>
	ID3D11Buffer* SetConstantBuffer(ID3D11Buffer* buffer, T writeData);
	template <typename T>
	ID3D11Buffer* SetConstantBuffer(ID3D11Buffer* buffer, T* writeData, int count);
};

template<class T>
inline void RenderState::RegisterLayout(std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutElements, const std::vector<BYTE> vsCode)
{
	m_layoutManager.RegisterLayout<T>(inputLayoutElements, vsCode, m_device);
}

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

// TODO if this works delete the method above and make the default value for count = 1
template<typename T>
inline ID3D11Buffer* RenderState::SetConstantBuffer(ID3D11Buffer* buffer, T* writeData, int count)
{
	D3D11_MAPPED_SUBRESOURCE res;
	auto hres = m_device.context()->Map((buffer), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, &writeData, count * sizeof(T));
	m_device.context()->Unmap(buffer, 0);
	ID3D11Buffer* cbs = buffer;
	return cbs;
}


//template ID3D11Buffer* RenderState::SetConstantBuffer<DirectX::XMVECTOR>(ID3D11Buffer* buffer, DirectX::XMVECTOR* writeData, int count);
//template ID3D11Buffer* RenderState::SetConstantBuffer<DirectX::XMMATRIX>(ID3D11Buffer* buffer, DirectX::XMMATRIX* writeData, int count);
//template ID3D11Buffer* RenderState::SetConstantBuffer<DirectX::XMVECTOR>(ID3D11Buffer* buffer, DirectX::XMVECTOR* writeData, int count);
//
//template ID3D11Buffer* RenderState::SetConstantBuffer<DirectX::XMMATRIX>(ID3D11Buffer* buffer, DirectX::XMMATRIX writeData);
//template ID3D11Buffer* RenderState::SetConstantBuffer<DirectX::XMVECTOR>(ID3D11Buffer* buffer, DirectX::XMVECTOR writeData);
