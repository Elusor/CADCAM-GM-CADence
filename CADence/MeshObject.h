#pragma once
#include <vector>
#include <d3d11.h>
#include "vertexStructures.h"
#include "Object.h"

struct MeshDescription
{
	D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology;
	std::vector<VertexPositionColor> vertices;
	std::vector<unsigned short> indices;
};

struct MeshObject : Object
{
	MeshDescription m_meshDesc;
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
};