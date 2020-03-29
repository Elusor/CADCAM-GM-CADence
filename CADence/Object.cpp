#include "Object.h"
#include "imgui.h"
#include "vertexStructures.h"
#include "Node.h"
using namespace DirectX;

bool Object::CreateParamsGui()
{
	bool objectChanged = false;
	float dragSpeed = 0.01f;
	float maxVal = 1000.0f;
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());
	ImGui::Spacing();


	std::string posScale = "##Position" + GetIdentifier();
	ImGui::Text("Position (x,y,z)");
	DirectX::XMFLOAT3 pos = m_transform.GetPosition();
	float posf[3] = { pos.x,pos.y,pos.z };
	objectChanged |= ImGui::DragFloat3(posScale.c_str(), (posf), dragSpeed, -maxVal, maxVal);
	m_transform.SetPosition(posf[0], posf[1], posf[2]);

	std::string rotScale = "##Rotation" + GetIdentifier();
	ImGui::Text("Rotation (x,y,z)");
	DirectX::XMFLOAT3 rot = m_transform.GetRotation();
	float rotf[3] = { rot.x,rot.y,rot.z };
	objectChanged |= ImGui::DragFloat3(rotScale.c_str(), (rotf), dragSpeed, -maxVal, maxVal);
	m_transform.SetRotation(rotf[0], rotf[1], rotf[2]);

	std::string labelScale = "##Scale" + GetIdentifier();
	ImGui::Text("Scale (x,y,z)");
	DirectX::XMFLOAT3 scale = m_transform.GetScale();
	float scalef[3] = { scale.x,scale.y,scale.z };
	objectChanged |= ImGui::DragFloat3(labelScale.c_str(), (scalef), dragSpeed, -maxVal, maxVal);
	m_transform.SetScale(scalef[0], scalef[1], scalef[2]);

	return objectChanged;
}

void Object::SetModified(bool value)
{
	m_modified = value;
}

bool Object::GetIsModified()
{
	return m_modified;
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
		{{0.0f,0.0f,0.0f},{0.0f,0.5f,1.0f}},
		{{0.0f,0.0f,5.0f},{0.0f,0.5f,1.0f}}
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

std::string Object::GetLabel()
{
	return m_name + "##" + m_defaultName;
}

std::string Object::GetIdentifier()
{
	return "##" + m_defaultName;
}

Transform& Object::GetTransform()
{
	return m_transform;
}

void Object::SetTransform(Transform transform)
{
	m_transform = transform;
	SetModified(true);
}

void Object::Translate(DirectX::XMFLOAT3 position)
{
	m_transform.Translate(position);
	SetModified(true);
}

void Object::Rotate(DirectX::XMFLOAT3 rotation)
{
	m_transform.Rotate(rotation);
	SetModified(true);
}

void Object::Scale(DirectX::XMFLOAT3 scale)
{
	m_transform.Scale(scale);
	SetModified(true);
}

void Object::SetPosition(DirectX::XMFLOAT3 position)
{
	m_transform.SetPosition(position);
	SetModified(true);
}

void Object::SetRotation(DirectX::XMFLOAT3 rotation)
{
	m_transform.SetRotation(rotation);
	SetModified(true);
}

void Object::SetScale(DirectX::XMFLOAT3 scale)
{
	m_transform.SetScale(scale);
	SetModified(true);
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
