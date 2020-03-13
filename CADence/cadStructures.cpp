#include "cadStructures.h"
#include "imgui.h"
#include "torusGenerator.h"
#include <d3d11.h>
#include "renderData.h"
using namespace DirectX;

bool SurfaceObject::CreateParamsGui()
{
	bool objectChanged = Object::CreateParamsGui();
	bool surfaceObjectChanged = false;
	surfaceObjectChanged |= ImGui::SliderInt("Density X", &(m_surParams.densityX), m_surParams.minDensityX, m_surParams.maxDensityX);
	surfaceObjectChanged |= ImGui::SliderInt("Density Y", &(m_surParams.densityY), m_surParams.minDensityY, m_surParams.maxDensityY);	
	return surfaceObjectChanged;
}

void SurfaceObject::RenderObject(RenderData* renderData)
{
	renderData->m_device.context()->IASetPrimitiveTopology(m_surVerDesc.m_primitiveTopology);

	//Update content to fill constant buffer
	D3D11_MAPPED_SUBRESOURCE res;
	XMMATRIX mvp = m_transform.GetModelMatrix() * renderData->m_camera->GetViewProjectionMatrix();
	//Set constant buffer
	auto hres = renderData->m_device.context()->Map((renderData->m_cbMVP.get()), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	renderData->m_device.context()->Unmap(renderData->m_cbMVP.get(), 0);
	ID3D11Buffer* cbs[] = { renderData->m_cbMVP.get() };
	renderData->m_device.context()->VSSetConstantBuffers(0, 1, cbs);

	// Update Vertex and index buffers
	renderData->m_vertexBuffer = (renderData->m_device.CreateVertexBuffer(m_surVerDesc.vertices));
	renderData->m_indexBuffer = (renderData->m_device.CreateIndexBuffer(m_surVerDesc.indices));
	ID3D11Buffer* vbs[] = { renderData->m_vertexBuffer.get() };

	//Update strides and offets based on the vertex class
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };

	renderData->m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Watch out for meshes that cannot be covered by ushort
	renderData->m_device.context()->IASetIndexBuffer(renderData->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	renderData->m_device.context()->DrawIndexed(m_surVerDesc.indices.size(), 0, 0);
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

void Torus::UpdateObject()
{
	GetTorusVerticesLineList(this);
}

//TODO [MG]: Use the object information to set up vertex and index buffers and call draw
void Object::RenderObject(RenderData* renderdata)
{
}

//TODO [MG]: Update the vertex and index info on the object if the object parameters have changed from gui or are transforming over time
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
