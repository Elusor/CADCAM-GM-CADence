#include <vector>
#include <d3d11.h>
#include "vertexStructures.h"

std::vector<D3D11_INPUT_ELEMENT_DESC> VertexPositionColor::GetInputLayoutElements()
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> elems{
		{
			"POSITION", 0,
			DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D11_INPUT_PER_VERTEX_DATA, 0
		},
		{
			"COLOR", 0,
			DXGI_FORMAT_R32G32B32_FLOAT, 0,
			offsetof(VertexPositionColor, color),
			D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};
	return elems;
}

std::vector<D3D11_INPUT_ELEMENT_DESC> VertexParameterColor::GetInputLayoutElements()
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> elems{
		{
			"POSITION", 0,
			DXGI_FORMAT_R32G32_FLOAT, 0, 0,
			D3D11_INPUT_PER_VERTEX_DATA, 0
		},
		{
			"COLOR", 0,
			DXGI_FORMAT_R32G32B32_FLOAT, 0,
			offsetof(VertexParameterColor, color),
			D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};
	return elems;
}
