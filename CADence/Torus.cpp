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

DirectX::XMFLOAT3 Torus::GetPoint(float u, float v)
{
	float x = (m_smallR * cosf(u) + m_bigR) * cosf(v);
	float y = m_smallR * sinf(u);
	float z = (m_smallR * cosf(u) + m_bigR) * sinf(v);

	return XMFLOAT3(x,y,z);
}

DirectX::XMFLOAT3 Torus::GetTangent(float u, float v, TangentDir tangentDir)
{
	float x, y, z;
	
	if (tangentDir == TangentDir::AlongU) {
		x = -(m_smallR * sinf(u)) * cosf(v);
		y = m_smallR * sinf(u);
		z = -(m_smallR * sinf(u)) * sinf(v);
	}
	else {
		x = -(m_smallR * cosf(u) + m_bigR) * sinf(v);
		y = 0.f;
		z = (m_smallR * cosf(u) + m_bigR) * cosf(v);
	}

	return XMFLOAT3(x, y, z);
}