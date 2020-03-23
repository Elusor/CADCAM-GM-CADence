#include "Object.h"
#include "imgui.h"
#include "vertexStructures.h"

using namespace DirectX;

bool Object::CreateParamsGui()
{
	bool objectChanged = false;
	float dragSpeed = 0.01f;
	float maxVal = 1000.0f;
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());
	ImGui::Spacing();

	std::string posX = "Position X##" + m_defaultName;
	std::string posY = "Position Y##" + m_defaultName;
	std::string posZ = "Position Z##" + m_defaultName;
	objectChanged |= ImGui::DragFloat(posX.c_str(), &(m_transform.m_pos.x), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(posY.c_str(), &(m_transform.m_pos.y), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(posZ.c_str(), &(m_transform.m_pos.z), dragSpeed, -maxVal, maxVal);
	ImGui::Spacing();

	std::string rotX = "Rotation X##" + m_defaultName;
	std::string rotY = "Rotation Y##" + m_defaultName;
	std::string rotZ = "Rotation Z##" + m_defaultName;
	objectChanged |= ImGui::DragFloat(rotX.c_str(), &(m_transform.m_rotation.x), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(rotY.c_str(), &(m_transform.m_rotation.y), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(rotZ.c_str(), &(m_transform.m_rotation.z), dragSpeed, -maxVal, maxVal);
	ImGui::Spacing();

	std::string scaleX = "Scale X##" + m_defaultName;
	std::string scaleY = "Scale Y##" + m_defaultName;
	std::string scaleZ = "Scale Z##" + m_defaultName;
	objectChanged |= ImGui::DragFloat(scaleX.c_str(), &(m_transform.m_scale.x), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(scaleY.c_str(), &(m_transform.m_scale.y), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(scaleZ.c_str(), &(m_transform.m_scale.z), dragSpeed, -maxVal, maxVal);
	ImGui::Spacing();

	return objectChanged;
}

void Object::RenderObject(std::unique_ptr<RenderState>& renderState)
{
}

void Object::RenderCoordinates(std::unique_ptr<RenderState>& renderData)
{
	//Update content to fill constant buffer
	D3D11_MAPPED_SUBRESOURCE res;
	XMMATRIX mvp = m_transform.GetModelMatrix() * renderData->m_camera->GetViewProjectionMatrix();
	//Set constant buffer
	auto hres = renderData->m_device.context()->Map((renderData->m_cbMVP.get()), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	renderData->m_device.context()->Unmap(renderData->m_cbMVP.get(), 0);
	ID3D11Buffer* cbs[] = { renderData->m_cbMVP.get() };
	renderData->m_device.context()->VSSetConstantBuffers(0, 1, cbs);

	std::vector<VertexPositionColor> vertices{
		{{0.0f,0.0f,0.0f},{1.0f,0.0f,0.0f}},
		{{5.0f,0.0f,0.0f},{1.0f,0.0f,0.0f}},
		{{0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f}},
		{{0.0f,5.0f,0.0f},{0.0f,1.0f,0.0f}},
		{{0.0f,0.0f,0.0f},{0.0f,0.0f,1.0f}},
		{{0.0f,0.0f,5.0f},{0.0f,0.0f,1.0f}}
	};

	std::vector<unsigned short> indices{
	0,1,
	2,3,
	4,5
	};

	renderData->m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Update Vertex and index buffers
	renderData->m_vertexBuffer = (renderData->m_device.CreateVertexBuffer(vertices));
	renderData->m_indexBuffer = (renderData->m_device.CreateIndexBuffer(indices));
	ID3D11Buffer* vbs[] = { renderData->m_vertexBuffer.get() };

	//Update strides and offets based on the vertex class
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };

	renderData->m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Watch out for meshes that cannot be covered by ushort
	renderData->m_device.context()->IASetIndexBuffer(renderData->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	renderData->m_device.context()->DrawIndexed(6, 0, 0);
}

void Object::UpdateObject()
{
}
