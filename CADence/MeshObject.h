#pragma once
#include <vector>
#include <d3d11.h>
#include <typeindex>
#include "vertexStructures.h"
#include "Object.h"
#include "imgui.h"

template <class T>
class MeshDescription
{
public:
	D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology;
	D3D11_PRIMITIVE_TOPOLOGY m_fillTopology;
	std::vector<T> vertices;
	std::vector<unsigned short> indices;
	std::vector<unsigned short> fillIndices;
	DirectX::XMFLOAT3 m_defaultColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	DirectX::XMFLOAT3 m_selectedColor = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f);
	DirectX::XMFLOAT3 m_adjustableColor = m_defaultColor;

	std::type_index GetVertexDataTypeIdx() { return std::type_index(typeid(T)); }
};

template <class T>
class MeshObject : public Object
{
public:
	virtual ~MeshObject() = default;
	MeshDescription<T> m_meshDesc;

	void SetIsSelected(bool isSelected) override;
	template <class K>
	void RenderMesh(std::unique_ptr<RenderState>& renderState, MeshDescription<K> desc);
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
};

template<class T>
template<class K>
inline void MeshObject<T>::RenderMesh(std::unique_ptr<RenderState>& renderState, MeshDescription<K> desc)
{
	renderState->m_device.context()->IASetPrimitiveTopology(desc.m_primitiveTopology);
	auto inputLayout = renderState->GetLayout(desc.GetVertexDataTypeIdx());
	renderState->m_device.context()->IASetInputLayout(inputLayout);

	//Set constant buffer
	XMMATRIX m = m_transform.GetModelMatrix();
	auto Mbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbM.get(), m);
	ID3D11Buffer* cbs1[] = { Mbuffer }; //, VPbuffer
	renderState->m_device.context()->VSSetConstantBuffers(1, 1, cbs1);

	// Update Vertex and index buffers
	renderState->m_vertexBuffer = (renderState->m_device.CreateVertexBuffer(desc.vertices));
	renderState->m_indexBuffer = (renderState->m_device.CreateIndexBuffer(desc.indices));
	ID3D11Buffer* vbs[] = { renderState->m_vertexBuffer.get() };

	//Update strides and offets based on the vertex class
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };

	renderState->m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Watch out for meshes that cannot be covered by ushort
	renderState->m_device.context()->IASetIndexBuffer(renderState->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	renderState->m_device.context()->DrawIndexed(desc.indices.size(), 0, 0);
}

template <class T>
inline void MeshObject<T>::SetIsSelected(bool isSelected)
{
	if (isSelected)
	{
		// object selected
		m_meshDesc.m_defaultColor = m_meshDesc.m_selectedColor;
		SetModified(true);
	}
	else
	{
		// object deselected
		m_meshDesc.m_defaultColor = m_meshDesc.m_adjustableColor;
		SetModified(true);
	}

	m_isSelected = isSelected;
}

template <class T>
inline void MeshObject<T>::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	RenderMesh(renderState, m_meshDesc);
}

template <class T>
inline bool MeshObject<T>::CreateParamsGui()
{
	bool meshChanged = false;
	meshChanged |= Object::CreateParamsGui();

	//change color
	float color[3] = {
		m_meshDesc.m_adjustableColor.x,
		m_meshDesc.m_adjustableColor.y,
		m_meshDesc.m_adjustableColor.z,
	};

	std::string text = "Default mesh color";
	ImGui::Text(text.c_str());
	meshChanged = ImGui::ColorEdit3(GetIdentifier().c_str(), (float*)&color);

	m_meshDesc.m_adjustableColor.x = color[0];
	m_meshDesc.m_adjustableColor.y = color[1];
	m_meshDesc.m_adjustableColor.z = color[2];

	return meshChanged;
}