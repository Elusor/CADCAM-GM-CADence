#include "MeshObject.h"
#include "imgui.h"
using namespace DirectX;

void MeshObject::RenderMesh(std::unique_ptr<RenderState>& renderState, MeshDescription desc)
{
	renderState->m_device.context()->IASetPrimitiveTopology(desc.m_primitiveTopology);
	//Update content to fill constant buffer
	D3D11_MAPPED_SUBRESOURCE res;
	XMMATRIX mvp = m_transform.GetModelMatrix() * renderState->m_camera->GetViewProjectionMatrix();
	//Set constant buffer
	auto hres = renderState->m_device.context()->Map((renderState->m_cbMVP.get()), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	renderState->m_device.context()->Unmap(renderState->m_cbMVP.get(), 0);
	ID3D11Buffer* cbs[] = { renderState->m_cbMVP.get() };
	renderState->m_device.context()->VSSetConstantBuffers(0, 1, cbs);

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

void MeshObject::RenderObject(std::unique_ptr<RenderState>& renderData)
{
	RenderMesh(renderData, m_meshDesc);
	//renderData->m_device.context()->IASetPrimitiveTopology(m_meshDesc.m_primitiveTopology);
	////Update content to fill constant buffer
	//D3D11_MAPPED_SUBRESOURCE res;
	//XMMATRIX mvp = m_transform.GetModelMatrix() * renderData->m_camera->GetViewProjectionMatrix();
	////Set constant buffer
	//auto hres = renderData->m_device.context()->Map((renderData->m_cbMVP.get()), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	//memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	//renderData->m_device.context()->Unmap(renderData->m_cbMVP.get(), 0);
	//ID3D11Buffer* cbs[] = { renderData->m_cbMVP.get() };
	//renderData->m_device.context()->VSSetConstantBuffers(0, 1, cbs);

	//// Update Vertex and index buffers
	//renderData->m_vertexBuffer = (renderData->m_device.CreateVertexBuffer(m_meshDesc.vertices));
	//renderData->m_indexBuffer = (renderData->m_device.CreateIndexBuffer(m_meshDesc.indices));
	//ID3D11Buffer* vbs[] = { renderData->m_vertexBuffer.get() };

	////Update strides and offets based on the vertex class
	//UINT strides[] = { sizeof(VertexPositionColor) };
	//UINT offsets[] = { 0 };

	//renderData->m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	//// Watch out for meshes that cannot be covered by ushort
	//renderData->m_device.context()->IASetIndexBuffer(renderData->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	//renderData->m_device.context()->DrawIndexed(m_meshDesc.indices.size(), 0, 0);
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
