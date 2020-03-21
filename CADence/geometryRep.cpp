#include "mathStructures.h"
#include <vector>

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
