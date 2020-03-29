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
	DirectX::XMFLOAT3 m_defaultColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
};

struct MeshObject : Object
{
	MeshDescription m_meshDesc;
	virtual void RenderMesh(std::unique_ptr<RenderState>& renderState, MeshDescription desc);
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
};