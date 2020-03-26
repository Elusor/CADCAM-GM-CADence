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

	DirectX::XMFLOAT3 pos = m_transform.GetPosition();
	DirectX::XMFLOAT3 rot = m_transform.GetRotation();
	DirectX::XMFLOAT3 scale = m_transform.GetScale();

	objectChanged |= ImGui::DragFloat(posX.c_str(), &(pos.x), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(posY.c_str(), &(pos.y), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(posZ.c_str(), &(pos.z), dragSpeed, -maxVal, maxVal);
	ImGui::Spacing();

	std::string rotX = "Rotation X##" + m_defaultName;
	std::string rotY = "Rotation Y##" + m_defaultName;
	std::string rotZ = "Rotation Z##" + m_defaultName;
	objectChanged |= ImGui::DragFloat(rotX.c_str(), &(rot.x), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(rotY.c_str(), &(rot.y), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(rotZ.c_str(), &(rot.z), dragSpeed, -maxVal, maxVal);
	ImGui::Spacing();

	std::string scaleX = "Scale X##" + m_defaultName;
	std::string scaleY = "Scale Y##" + m_defaultName;
	std::string scaleZ = "Scale Z##" + m_defaultName;
	objectChanged |= ImGui::DragFloat(scaleX.c_str(), &(scale.x), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(scaleY.c_str(), &(scale.y), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(scaleZ.c_str(), &(scale.z), dragSpeed, -maxVal, maxVal);
	ImGui::Spacing();
	
	m_transform.SetPosition(pos);
	m_transform.SetRotation(rot);
	m_transform.SetScale(scale);

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

void Object::RenderObjectSpecificContextOptions(Scene& scene)
{
}

void Object::UpdateObject()
{
}

#pragma region Transform Wrappers

Transform& Object::GetTransform()
{
	return m_transform;
}

void Object::SetTransform(Transform transform)
{
	m_transform = transform;
}

void Object::Translate(DirectX::XMFLOAT3 position)
{
	m_transform.Translate(position);
}

void Object::Rotate(DirectX::XMFLOAT3 rotation)
{
	m_transform.Rotate(rotation);
}

void Object::Scale(DirectX::XMFLOAT3 scale)
{
	m_transform.Scale(scale);
}

void Object::SetPosition(DirectX::XMFLOAT3 position)
{
	m_transform.SetPosition(position);
}

void Object::SetRotation(DirectX::XMFLOAT3 rotation)
{
	m_transform.SetRotation(rotation);
}

void Object::SetScale(DirectX::XMFLOAT3 scale)
{
	m_transform.SetScale(scale);
}

DirectX::XMFLOAT3 Object::GetPosition()
{
	return m_transform.GetPosition();
}

DirectX::XMFLOAT3 Object::GetRotation()
{
	return m_transform.GetRotation();
}

DirectX::XMFLOAT3 Object::GetScale()
{
	return m_transform.GetScale();
}

#pragma endregion
