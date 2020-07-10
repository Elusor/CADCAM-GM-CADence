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

	auto relativePos = XMFLOAT3(x, y, z);
	auto scenePos = GetPosition() + relativePos;

	return scenePos;
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

DirectX::XMFLOAT3 Torus::GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir)
{
	float x, y, z;

	switch (tangentDir)
	{
	case TangentDir::AlongU:
		x = -m_smallR * cosf(u) * cosf(v);
		y = -m_smallR * sinf(u);
		z = -m_smallR * cosf(u) * sinf(v);
		break;
	case TangentDir::AlongV:
		x = -(m_smallR * cosf(u) + m_bigR) * cosf(v);
		y = 0.f;
		z = -(m_smallR * cosf(u) + m_bigR) * sinf(v);
		break;
	}

	return DirectX::XMFLOAT3(x, y, z);
}

DirectX::XMFLOAT3 Torus::GetSecondDarivativeMixed(float u, float v)
{
	float x, y, z;

	x = m_smallR * sinf(u) * sinf(v);
	y = 0;
	z = -m_smallR * sinf(u) * cosf(v);

	return DirectX::XMFLOAT3(x,y,z);
}

bool Torus::ParamsInsideBounds(float u, float v)
{
	// Torus can be looped in both dimensions so parameters can never be out of bounds
	return true;
}

void Torus::GetWrappedParams(float& u, float& v)
{
	float uIntPart;
	float newU = modff(u, &uIntPart);

	float vIntPart;
	float newV = modff(v, &vIntPart);

	if (newU < 0.f)
	{
		newU = 1 + newU;
	}

	if (newV < 0.f)
	{
		newV = 1 + newV;
	}

	u = newU;
	v = newV;
}

float Torus::GetFarthestPointInDirection(float u, float v, DirectX::XMFLOAT2 dir, float defStep)
{
	return defStep;
}
