#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>

class VertexData {};

struct VertexPositionColor : VertexData
{
	VertexPositionColor(DirectX::XMFLOAT3 _pos, DirectX::XMFLOAT3 _col) : position(_pos), color(_col) {};
	DirectX::XMFLOAT3 position, color;
	static std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayoutElements();
};

struct VertexParameterColor : VertexData
{
	VertexParameterColor(DirectX::XMFLOAT2 _par, DirectX::XMFLOAT3 _col) : parameters(_par), color(_col) {};
	DirectX::XMFLOAT2 parameters;
	DirectX::XMFLOAT3 color;
	static std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayoutElements();
};


struct ConstantParameters
{
	DirectX::XMFLOAT2 uv;
	static std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayoutElements();
};
