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
	std::string dtd = "##Debug tangents display" + GetIdentifier();


	ImGui::Text("Main radius");
	torusChanged |= ImGui::SliderFloat(mainR.c_str(), &(torus->m_bigR), 0.0f, 15.0f);
	ImGui::Text("Secondary radius");
	torusChanged |= ImGui::SliderFloat(secR.c_str(), &(torus->m_smallR), 0.0f, 15.0f);

	bool debugTangents = m_displayTangents;
	ImGui::Text("Display tangents");
	torusChanged |= ImGui::Checkbox(dtd.c_str(), &debugTangents);

	m_displayTangents = debugTangents;

	ImGui::End();

	return torusChanged;
}

void Torus::UpdateObject()
{
	GetTorusVerticesLineList(this);
	m_debugDesc.vertices.clear();
	m_debugDesc.indices.clear();

	m_debugDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	float length = 0.25f;
	int i = 0;
	for (int u = 0; u <= 25; u++)
	{
		for (int v = 0; v <= 25; v++)
		{
			float uParam = (float)u / 25.f;
			float vParam = (float)v / 25.f;

			auto pt = GetPoint(uParam, vParam);
			auto derU = this->GetTangent(uParam, vParam, TangentDir::AlongU);
			auto derV = this->GetTangent(uParam, vParam, TangentDir::AlongV);

			auto ptU = pt + length * derU;
			auto ptV = pt + length * derV;

			m_debugDesc.vertices.push_back(
				{
					{pt},
					{1.0f, 0.0f, 0.0f}
				});

			m_debugDesc.vertices.push_back(
				{
					{ptU},
					{1.0f, 0.0f, 0.0f}
				});

			m_debugDesc.indices.push_back(i);
			m_debugDesc.indices.push_back(i+1);
			i += 2;

			m_debugDesc.vertices.push_back(
				{
					{pt},
					{0.0f, 1.0f, 0.0f}
				});

			m_debugDesc.vertices.push_back(
				{
					{ptV},
					{0.0f, 1.0f, 0.0f}
				});

			m_debugDesc.indices.push_back(i);
			m_debugDesc.indices.push_back(i + 1);
			i += 2;
		}
	}
}

void Torus::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	RenderMesh(renderState, m_meshDesc);
	if(m_displayTangents)
		RenderMesh(renderState, m_debugDesc);
}

DirectX::XMFLOAT3 Torus::GetPoint(float u, float v)
{
	u *= XM_2PI;
	v *= XM_2PI;

	float x = (m_smallR * cosf(u) + m_bigR) * cosf(v);
	float y = m_smallR * sinf(u);
	float z = (m_smallR * cosf(u) + m_bigR) * sinf(v);

	auto relativePos = XMFLOAT4(x, y, z, 1.0f);
	auto mtx = m_transform.GetModelMatrix();
	DirectX::XMFLOAT4X4 modelMtx;
	DirectX::XMStoreFloat4x4(&modelMtx, mtx);
	auto pos = Mul(relativePos, modelMtx);

	return DirectX::XMFLOAT3(pos.x, pos.y, pos.z);
}

DirectX::XMFLOAT3 Torus::GetTangent(float u, float v, TangentDir tangentDir)
{
	float x, y, z;
	
	u *= XM_2PI;
	v *= XM_2PI;

	if (tangentDir == TangentDir::AlongU) {
		x = -(m_smallR * sinf(u)) * cosf(v);
		y = m_smallR * cosf(u);
		z = -(m_smallR * sinf(u)) * sinf(v);
	}
	else {
		x = -(m_smallR * cosf(u) + m_bigR) * sinf(v);
		y = 0.f;
		z = (m_smallR * cosf(u) + m_bigR) * cosf(v);
	}
	
	auto relativePos = XMFLOAT4(x, y, z, 0.0f);
	auto mtx = m_transform.GetModelMatrix();
	DirectX::XMFLOAT4X4 modelMtx;
	DirectX::XMStoreFloat4x4(&modelMtx, mtx);
	auto pos = Mul(relativePos, modelMtx);

	return DirectX::XMFLOAT3(pos.x, pos.y, pos.z);
}

DirectX::XMFLOAT3 Torus::GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir)
{
	float x, y, z;

	u *= XM_2PI;
	v *= XM_2PI;

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

	auto relativePos = XMFLOAT4(x, y, z, 0.0f);
	auto mtx = m_transform.GetModelMatrix();
	DirectX::XMFLOAT4X4 modelMtx;
	DirectX::XMStoreFloat4x4(&modelMtx, mtx);
	auto pos = Mul(relativePos, modelMtx);

	return DirectX::XMFLOAT3(pos.x, pos.y, pos.z);
}

DirectX::XMFLOAT3 Torus::GetSecondDarivativeMixed(float u, float v)
{
	float x, y, z;

	u *= XM_2PI;
	v *= XM_2PI;

	x = m_smallR * sinf(u) * sinf(v);
	y = 0;
	z = -m_smallR * sinf(u) * cosf(v);

	auto relativePos = XMFLOAT4(x, y, z, 0.0f);
	auto mtx = m_transform.GetModelMatrix();
	DirectX::XMFLOAT4X4 modelMtx;
	DirectX::XMStoreFloat4x4(&modelMtx, mtx);
	auto pos = Mul(relativePos, modelMtx);

	return DirectX::XMFLOAT3(pos.x, pos.y, pos.z);
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
