#include "MeshObject.h"
#include "imgui.h"
using namespace DirectX;

void MeshObject::RenderMesh(std::unique_ptr<RenderState>& renderState, MeshDescription desc)
{
	renderState->m_device.context()->IASetPrimitiveTopology(desc.m_primitiveTopology);

	//Set constant buffer
	XMMATRIX m = m_transform.GetModelMatrix();
	auto Mbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbM.get(), m);
	ID3D11Buffer* cbs1[] = { Mbuffer}; //, VPbuffer
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

void MeshObject::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	RenderMesh(renderState, m_meshDesc);
}

bool MeshObject::CreateParamsGui()
{
	bool meshChanged = false;
	meshChanged |= Object::CreateParamsGui();
	
	//change color
	float color[3] = {
		m_meshDesc.m_defaultColor.x,
		m_meshDesc.m_defaultColor.y,
		m_meshDesc.m_defaultColor.z,
	};

	std::string text = "Default mesh color";
	ImGui::Text(text.c_str());
	meshChanged = ImGui::ColorEdit3(GetIdentifier().c_str(), (float*)&color);
	
	m_meshDesc.m_defaultColor.x = color[0];
	m_meshDesc.m_defaultColor.y = color[1];
	m_meshDesc.m_defaultColor.z = color[2];
	
	return meshChanged;
}
