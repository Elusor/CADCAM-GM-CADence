#include "ParametrizedObject2D.h"
#include "imgui.h"

bool ParametrizedObject2D::CreateParamsGui()
{
	bool surfaceObjectChanged = false;
	surfaceObjectChanged |= MeshObject::CreateParamsGui();
	ImGui::Spacing();
	std::string mainR = "##Density U" + GetIdentifier();
	std::string secR = "##Density V" +  GetIdentifier();

	ImGui::Text("Density U");
	surfaceObjectChanged |= ImGui::SliderInt(mainR.c_str(), &(m_surParams.densityX), m_surParams.minDensityX, m_surParams.maxDensityX);
	ImGui::Text("Density V");
	surfaceObjectChanged |= ImGui::SliderInt(secR.c_str(), &(m_surParams.densityY), m_surParams.minDensityY, m_surParams.maxDensityY);
	return surfaceObjectChanged;
}