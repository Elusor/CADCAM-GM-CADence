#include "cadStructures.h"
#include "imgui.h"
#include "torusGenerator.h"

bool SurfaceObject::CreateObjectsImguiSection()
{
	bool objectChanged = false;
	objectChanged |= ImGui::SliderInt("Density X", &(m_surParams.densityX), m_surParams.minDensityX, m_surParams.maxDensityX);
	objectChanged |= ImGui::SliderInt("Density Y", &(m_surParams.densityY), m_surParams.minDensityY, m_surParams.maxDensityY);

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

bool Torus::CreateObjectsImguiSection()
{	
	SurfaceParametrizationParams* surParams = &(this->m_surParams);
	ImGui::Begin("Torus parameters");
	ImGui::Text("Sliders describing the density of the mesh:");
	// Create sliders for torus parameters	
	Torus* torus = this;
	bool torusChanged = SurfaceObject::CreateObjectsImguiSection();
	
	torusChanged |= ImGui::SliderFloat("Main radius", &(torus->m_bigR), 0.0f, 15.0f);
	torusChanged |= ImGui::SliderFloat("Secondary radius", &(torus->m_smallR), 0.0f, 15.0f);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	return torusChanged;
}

void Torus::RenderObject()
{
	/*D3D11_MAPPED_SUBRESOURCE res;
	XMStoreFloat4x4(&m_viewMat, m_camera->GetViewMatrix());
	XMStoreFloat4x4(&m_modelMat, m_surObj->m_transform.GetModelMatrix());

	XMMATRIX mvp = XMLoadFloat4x4(&m_modelMat) * XMLoadFloat4x4(&m_viewMat) * XMLoadFloat4x4(&m_projMat);
	m_device.context()->Map(m_cbMVP.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	m_device.context()->Unmap(m_cbMVP.get(), 0);*/
	/*ID3D11Buffer* cbs[] = { m_cbMVP.get() };
	m_device.context()->VSSetConstantBuffers(0, 1, cbs);*/

	//m_device.context()->DrawIndexed(m_surObj->m_surVerDesc.indices.size(), 0, 0);

}

void Torus::UpdateObject()
{
	GetTorusVerticesLineList(this);
}

/// Use the object information to set up vertex and index buffers and call draw
void Object::RenderObject()
{
}

/// Update the vertex and index info on the object if the object parameters have changed from gui or are transforming over time
void Object::UpdateObject()
{
}
