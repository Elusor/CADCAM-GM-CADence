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
	DirectX::XMFLOAT3 m_selectedColor = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f);
	DirectX::XMFLOAT3 m_adjustableColor = m_defaultColor;
};

template <class T>
struct MeshObject : Object
{
	virtual ~MeshObject() = default;
	MeshDescription<T> m_meshDesc;

	void SetIsSelected(bool isSelected) override;
	virtual void RenderMesh(std::unique_ptr<RenderState>& renderState, MeshDescription<T> desc);
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
};