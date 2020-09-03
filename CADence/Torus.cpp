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
	torusChanged |= ImGui::SliderFloat(mainR.c_str(), &(torus->m_donutR), 0.0f, 15.0f);
	ImGui::Text("Secondary radius");
	torusChanged |= ImGui::SliderFloat(secR.c_str(), &(torus->m_tubeR), 0.0f, 15.0f);

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
	float length = 1.f;
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
			auto norm = Cross(derV,derU);

			auto ptU = pt + length * derU;
			auto ptV = pt + length * derV;
			auto ptNorm = pt + norm / sqrtf(Dot(norm,norm));

			XMFLOAT3 uCol = { 1.0f, uParam, uParam };
			XMFLOAT3 vCol = { vParam, 1.0f, vParam };
			XMFLOAT3 nCol = { 0.0f, 0.0f, 1.0f };

			m_debugDesc.vertices.push_back(
				{
					{pt},
					uCol
				});

			m_debugDesc.vertices.push_back(
				{
					{ptU},
					uCol
				});

			m_debugDesc.indices.push_back(i);
			m_debugDesc.indices.push_back(i+1);
			i += 2;

			m_debugDesc.vertices.push_back(
				{
					{pt},
					vCol
				});

			m_debugDesc.vertices.push_back(
				{
					{ptV},
					vCol
				});

			m_debugDesc.indices.push_back(i);
			m_debugDesc.indices.push_back(i + 1);
			i += 2;

			m_debugDesc.vertices.push_back(
				{
					{pt},
					nCol
				});

			m_debugDesc.vertices.push_back(
				{
					{ptNorm},
					nCol
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

ParameterPair Torus::GetMaxParameterValues()
{
	ParameterPair res;
	res.u = XM_2PI;
	res.v = XM_2PI;

	return res;
}

ParameterPair Torus::GetNormalizedParams(float u, float v)
{
	float newU = fmod(u, XM_2PI);
	float newV = fmod(v, XM_2PI);

	if (newU < 0.f)
	{
		newU = XM_2PI + newU;
	}

	if (newV < 0.f)
	{
		newV = XM_2PI + newV;
	}

	u = newU / XM_2PI;
	v = newV / XM_2PI;

	return ParameterPair(u,v);
}

DirectX::XMFLOAT3 Torus::GetPoint(float u, float v)
{
	float x = (m_donutR + m_tubeR * cosf(v)) * cosf(u);
	float y = m_tubeR * sinf(v);
	float z = (m_donutR + m_tubeR * cosf(v)) * sinf(u);

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

	if (tangentDir == TangentDir::AlongU) {
		x = -(m_donutR + m_tubeR * cosf(v)) * sinf(u);
		y = 0.0f;
		z = (m_donutR + m_tubeR * cosf(v)) * cosf(u);
	}
	else {
		x = -m_tubeR * sinf(v) * cosf(u);
		y = m_tubeR * cosf(v);
		z = -m_tubeR * sinf(v) * sinf(u);
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
		x = -m_tubeR * cosf(u) * cosf(v);
		y = -m_tubeR * sinf(u);
		z = -m_tubeR * cosf(u) * sinf(v);
		break;
	case TangentDir::AlongV:
		x = -(m_tubeR * cosf(u) + m_donutR) * cosf(v);
		y = 0.f;
		z = -(m_tubeR * cosf(u) + m_donutR) * sinf(v);
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

	x = m_tubeR * sinf(u) * sinf(v);
	y = 0;
	z = -m_tubeR * sinf(u) * cosf(v);

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

ParameterPair Torus::GetWrappedParams(float u, float v)
{
	// Fmod here causes the values to be inexact and give unsatisfying results. Just return the old values.
	return ParameterPair(u, v);
}

float Torus::GetFarthestPointInDirection(float u, float v, DirectX::XMFLOAT2 dir, float defStep)
{
	return defStep;
}
