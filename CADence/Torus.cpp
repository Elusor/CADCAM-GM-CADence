#include "Torus.h"
#include "imgui.h"
#include "torusGenerator.h"

bool Torus::CreateParamsGui()
{
	Parametrization2DArguments* surParams = &(this->m_surParams);
	// TODO [MG] generalize inspector creation
	ImGui::Begin("Inspector");
	// Create sliders for torus parameters	
	Torus* torus = this;
	bool torusChanged = ParametrizedObject2D::CreateParamsGui();
	ImGui::Spacing();

	std::string mainR = "##Main radius" + GetIdentifier();
	std::string secR = "##Secondary radius" + GetIdentifier();

	ImGui::Text("Main radius");
	torusChanged |= ImGui::SliderFloat(mainR.c_str(), &(torus->m_bigR), 0.0f, 15.0f);
	ImGui::Text("Secondary radius");
	torusChanged |= ImGui::SliderFloat(secR.c_str(), &(torus->m_smallR), 0.0f, 15.0f);

	ImGui::End();

	return torusChanged;
}

void Torus::UpdateObject()
{
	GetTorusVerticesLineList(this);
}