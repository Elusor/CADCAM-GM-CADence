#include "cadStructures.h"
#include "imgui.h"

bool SurfaceObject::CreateObjectsImguiSection()
{
	bool objectChanged = false;
	objectChanged |= ImGui::SliderInt("Density X", &(m_surParams.densityX), m_surParams.minDensityX, m_surParams.maxDensityX);
	objectChanged |= ImGui::SliderInt("Density Y", &(m_surParams.densityY), m_surParams.minDensityY, m_surParams.maxDensityY);

	ImGui::Spacing();
	objectChanged |= ImGui::SliderFloat("Position X", &(m_transform.m_pos.x), 0.005f, 5.0f);
	objectChanged |= ImGui::SliderFloat("Position Y", &(m_transform.m_pos.y), 0.005f, 5.0f);
	objectChanged |= ImGui::SliderFloat("Position Z", &(m_transform.m_pos.z), 0.005f, 5.0f);
	ImGui::Spacing();
	objectChanged |= ImGui::SliderFloat("Rotation X", &(m_transform.m_rotation.x), 0.005f, 5.0f);
	objectChanged |= ImGui::SliderFloat("Rotation Y", &(m_transform.m_rotation.y), 0.005f, 5.0f);
	objectChanged |= ImGui::SliderFloat("Rotation Z", &(m_transform.m_rotation.z), 0.005f, 5.0f);
	ImGui::Spacing();
	objectChanged |= ImGui::SliderFloat("Scale X", &(m_transform.m_scale.x), 0.005f, 5.0f);
	objectChanged |= ImGui::SliderFloat("Scale Y", &(m_transform.m_scale.y), 0.005f, 5.0f);
	objectChanged |= ImGui::SliderFloat("Scale Z", &(m_transform.m_scale.z), 0.005f, 5.0f);
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