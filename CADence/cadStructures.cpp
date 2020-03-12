#include "cadStructures.h"
#include "imgui.h"
#include "torusGenerator.h"
using namespace DirectX;
#include <d3d11.h>
#include "renderData.h"

bool SurfaceObject::CreateParamsGui()
{
	bool objectChanged = Object::CreateParamsGui();
	bool surfaceObjectChanged = false;
	surfaceObjectChanged |= ImGui::SliderInt("Density X", &(m_surParams.densityX), m_surParams.minDensityX, m_surParams.maxDensityX);
	surfaceObjectChanged |= ImGui::SliderInt("Density Y", &(m_surParams.densityY), m_surParams.minDensityY, m_surParams.maxDensityY);	
	return surfaceObjectChanged;
}

bool Torus::CreateParamsGui()
{	
	SurfaceParametrizationParams* surParams = &(this->m_surParams);
	ImGui::Begin("Torus parameters");
	ImGui::Text("Sliders describing the density of the mesh:");
	// Create sliders for torus parameters	
	Torus* torus = this;
	bool torusChanged = SurfaceObject::CreateParamsGui();
	
	torusChanged |= ImGui::SliderFloat("Main radius", &(torus->m_bigR), 0.0f, 15.0f);
	torusChanged |= ImGui::SliderFloat("Secondary radius", &(torus->m_smallR), 0.0f, 15.0f);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	return torusChanged;
}

void Torus::RenderObject(RenderData* renderData)
{	
	// camera
	// model matrix
	// m_device
	// vertex buffer 
	// index buffer
	// constant buffer
	//RenderData d = *renderData;
	//
	//// set up the right input layout 

	//D3D11_MAPPED_SUBRESOURCE res;

	//XMMATRIX mvp = m_transform.GetModelMatrix() * d.m_camera->GetViewProjectionMatrix();
	//auto hres = d.m_device->context()->Map(d.m_cbMVP, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	//memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	//d.m_device->context()->Unmap(d.m_cbMVP, 0);
	//ID3D11Buffer* cbs[] = { d.m_cbMVP };
	//d.m_device->context()->VSSetConstantBuffers(0, 1, cbs);
	//d.m_vertexBuffer = (d.m_device->CreateVertexBuffer(m_surVerDesc.vertices)).get();
	//d.m_indexBuffer = (d.m_device->CreateIndexBuffer(m_surVerDesc.indices)).get();
	//ID3D11Buffer* vbs[] = { d.m_vertexBuffer };
	//UINT strides[] = { sizeof(VertexPositionColor) };
	//UINT offsets[] = { 0 };	
	//d.m_device->context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	//d.m_device->context()->IASetIndexBuffer(d.m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	//d.m_device->context()->DrawIndexed(m_surVerDesc.indices.size(), 0, 0);
}

void Torus::UpdateObject()
{
	GetTorusVerticesLineList(this);
}

//void Object::AttachNode(Node* newNode)
//{
//	//TODO [MG]: if this object had an assigned node, make sure to delete the old node and move all the children to this node
//	this->m_node = newNode;
//}
//
//void Object::AttachChild(Object* obj)
//{
//	m_node->AttachChild(obj);
//}

/// Use the object information to set up vertex and index buffers and call draw
void Object::RenderObject(RenderData* renderdata)
{
}

/// Update the vertex and index info on the object if the object parameters have changed from gui or are transforming over time
void Object::UpdateObject()
{
}

bool Object::CreateParamsGui()
{
	bool objectChanged = false;
	float dragSpeed = 0.01f;

	ImGui::Spacing();
	objectChanged |= ImGui::DragFloat("Position X", &(m_transform.m_pos.x), dragSpeed, 0.005f, 5.0f);
	objectChanged |= ImGui::DragFloat("Position Y", &(m_transform.m_pos.y), dragSpeed, 0.005f, 5.0f);
	objectChanged |= ImGui::DragFloat("Position Z", &(m_transform.m_pos.z), dragSpeed, 0.005f, 5.0f);
	ImGui::Spacing();
	objectChanged |= ImGui::DragFloat("Rotation X", &(m_transform.m_rotation.x), dragSpeed, 0.005f, 5.0f);
	objectChanged |= ImGui::DragFloat("Rotation Y", &(m_transform.m_rotation.y), dragSpeed, 0.005f, 5.0f);
	objectChanged |= ImGui::DragFloat("Rotation Z", &(m_transform.m_rotation.z), dragSpeed, 0.005f, 5.0f);
	ImGui::Spacing();
	objectChanged |= ImGui::DragFloat("Scale X", &(m_transform.m_scale.x), dragSpeed, 0.005f, 5.0f);
	objectChanged |= ImGui::DragFloat("Scale Y", &(m_transform.m_scale.y), dragSpeed, 0.005f, 5.0f);
	objectChanged |= ImGui::DragFloat("Scale Z", &(m_transform.m_scale.z), dragSpeed, 0.005f, 5.0f);
	ImGui::Spacing();

	return objectChanged;
}
