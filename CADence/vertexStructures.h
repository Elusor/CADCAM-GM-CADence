#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>
struct VertexPositionColor 
{
	DirectX::XMFLOAT3 position, color;
	static std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayoutElements();
};

struct ConstantParameters
{
	DirectX::XMFLOAT2 uv;
	static std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayoutElements();
};
