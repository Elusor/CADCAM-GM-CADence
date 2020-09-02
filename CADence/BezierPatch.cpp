#include "BezierPatch.h"
#include "imgui.h"
#include "bezierCalculator.h"
#include "mathUtils.h"
#include "Scene.h"
#include "Transform.h"

BezierPatch::BezierPatch()
{
	m_displayPolygon = false;
	std::weak_ptr<Node> empty;
}

BezierPatch::~BezierPatch()
{
}

void BezierPatch::Initialize(
	std::vector<std::weak_ptr<Node>> first, 
	std::vector<std::weak_ptr<Node>> second, 
	std::vector<std::weak_ptr<Node>> third, 
	std::vector<std::weak_ptr<Node>> fourth)
{
	m_displayPolygon = false;
	
	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(first[i]);
	}

	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(second[i]);
	}

	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(third[i]);
	}

	for (int i = 0; i < 4; i++)
	{
		GetReferences().LinkRef(fourth[i]);
	}


	auto refs = GetReferences().GetAllRef();
	for (int i = 0; i < 16; i++)
	{
		assert(!refs[i].m_refered.expired());
	}
	UpdateObject();
}

void BezierPatch::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	RenderPatch(renderState);
	if (m_displayPolygon)
	{
		RenderPolygon(renderState);
	}
}

void BezierPatch::RenderPatch(std::unique_ptr<RenderState>& renderState)
{

	auto context = renderState->m_device.context().get();
	XMMATRIX x = GetCoordinates(Coord::Xpos);
	XMMATRIX y = GetCoordinates(Coord::Ypos);
	XMMATRIX z = GetCoordinates(Coord::Zpos);

	XMMATRIX mat = m_transform.GetModelMatrix();
	auto Mbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbM.get(), mat);
	ID3D11Buffer* cbs1[] = { Mbuffer }; //, VPbuffer
	renderState->m_device.context()->VSSetConstantBuffers(1, 1, cbs1);

	D3D11_RASTERIZER_DESC desc;
	desc.FillMode = D3D11_FILL_WIREFRAME;
	desc.CullMode = D3D11_CULL_NONE;
	desc.AntialiasedLineEnable = 0;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0;
	desc.DepthClipEnable = true;
	desc.FrontCounterClockwise = 0;
	desc.MultisampleEnable = 0;
	desc.ScissorEnable = 0;
	desc.SlopeScaledDepthBias = 0;

	ID3D11RasterizerState* rs;

	renderState->m_device.m_device->CreateRasterizerState(&desc, &rs);
	context->RSSetState(rs);

	context->HSSetShader(renderState->m_patchHullShader.get(), 0, 0);
	ID3D11Buffer* hsCb[] = { renderState->m_cbPatchDivisions.get() };
	context->HSSetConstantBuffers(0, 1, hsCb);

	context->DSSetShader(renderState->m_patchDomainShader.get(), 0, 0);
	context->DSSetConstantBuffers(1, 1, cbs1);
	ID3D11Buffer* cbs2[] = { renderState->m_cbVP.get() };
	context->DSSetConstantBuffers(0, 1, cbs2);

	XMFLOAT4 divs = XMFLOAT4(m_uSize, 0.0f, 0.f, 0.f);
	auto divBuff = renderState->SetConstantBuffer<XMFLOAT4>(renderState->m_cbPatchDivisions.get(), divs);
	ID3D11Buffer* divCBuffer[] = { divBuff }; //, VPbuffer
	renderState->m_device.context()->HSSetConstantBuffers(0, 1, divCBuffer);	
	MeshObject::RenderMesh(renderState, m_UDesc);

	//divs = XMFLOAT4(m_vSize, 0.0f, 0.f, 0.f);
	//divBuff = renderState->SetConstantBuffer<XMFLOAT4>(renderState->m_cbPatchDivisions.get(), divs);
	//ID3D11Buffer* divCBuffer2[] = { divBuff }; //, VPbuffer
	//renderState->m_device.context()->HSSetConstantBuffers(0, 1, divCBuffer2);
	//MeshObject::RenderMesh(renderState, m_VDesc);
	context->HSSetShader(nullptr, 0, 0);
	context->DSSetShader(nullptr, 0, 0);
	context->RSSetState(nullptr);
}

void BezierPatch::RenderPolygon(std::unique_ptr<RenderState>& renderState)
{
	RenderMesh(renderState, m_PolygonDesc);
}

bool BezierPatch::CreateParamsGui()
{
	ImGui::Begin("Inspector");
	// Create sliders for torus parameters	
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());	
	bool patchChanged = false;

	std::string label = "Display Bezier polygon" + GetIdentifier();
	patchChanged |= ImGui::Checkbox(label.c_str(), &m_displayPolygon);
	ImGui::Spacing();

	// change colors for polygon
	float pcolor[3] = {
		m_PolygonDesc.m_defaultColor.x,
		m_PolygonDesc.m_defaultColor.y,
		m_PolygonDesc.m_defaultColor.z,
	};

	std::string ptext = "Bezier color";
	ImGui::Text(ptext.c_str());
	patchChanged |= ImGui::ColorEdit3((ptext + "##" + ptext + GetIdentifier()).c_str(), (float*)&pcolor);

	m_PolygonDesc.m_defaultColor.x = pcolor[0];
	m_PolygonDesc.m_defaultColor.y = pcolor[1];
	m_PolygonDesc.m_defaultColor.z = pcolor[2];
	ImGui::Spacing();

	std::string dimDrag = "Grid density" + GetIdentifier();
	patchChanged |= ImGui::DragInt(dimDrag.c_str(), &m_uSize, 1.0f, 2, 64);

	if (m_uSize < 2)
		m_uSize = 2;
	if (m_uSize > 64)
		m_uSize = 64;
	
	m_vSize = m_uSize;

	ImGui::End();
	return patchChanged;
}

void BezierPatch::UpdateObject()
{	
	m_VDesc.m_primitiveTopology = m_UDesc.m_primitiveTopology = m_meshDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;

	// pass an (u,v) line to the shader
	// one of those is the constant parameter - the other one is 
	std::vector<VertexPositionColor> vertices;
	std::vector<unsigned short> indicesU;
	std::vector<unsigned short> indicesV; 

	for (int i = 0; i < 16; i++) {
		vertices.push_back(VertexPositionColor{
			GetReferences().GetAllRef()[i].m_refered.lock()->m_object->GetPosition(),
			m_meshDesc.m_defaultColor });
		indicesU.push_back(i);
	}

	/*for (int i = 0; i < 4; i++) {
		indicesV.push_back(i );
		indicesV.push_back(i + 4);
		indicesV.push_back(i + 8);
		indicesV.push_back(i + 12);
	}*/
	
	m_PolygonDesc.vertices.clear();
	m_PolygonDesc.indices.clear();
	m_PolygonDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

	int i = 0;
	float length = 0.09f;

	for (int u = 0; u <= 10; u++)
	{
		for (int v = 0; v <= 10; v++)
		{
			float uParam = (float)u / 10.f;
			float vParam = (float)v / 10.f;

			auto pt = this->GetPoint(uParam, vParam);
			auto derPt = this->GetTangent(uParam, vParam, TangentDir::AlongU);
			auto finalPt = pt + length * derPt;

			m_PolygonDesc.vertices.push_back(
				{
					{pt},
					{1.0f,uParam,uParam}
				});
			m_PolygonDesc.vertices.push_back(
				{
					{finalPt},
					{1.0f,uParam,uParam}
				});
			m_PolygonDesc.indices.push_back(i);
			m_PolygonDesc.indices.push_back(i+1);			
			i += 2;

			auto derPt2 = this->GetTangent(uParam, vParam, TangentDir::AlongV);
			auto finalPt2 = pt + length * derPt2;

			m_PolygonDesc.vertices.push_back(
				{
					{pt},
					{vParam,1.0f,vParam}
				});
			m_PolygonDesc.vertices.push_back(
				{
					{finalPt2},
					{vParam,1.0f,vParam}
				});
			m_PolygonDesc.indices.push_back(i);
			m_PolygonDesc.indices.push_back(i + 1);

			i += 2;

			/*auto derPt3 = this->GetSecondDarivativeMixed(uParam, vParam);
			auto finalPt3 = pt + length * derPt3;
			m_PolygonDesc.vertices.push_back(
				{
					{pt},
					{0.0f,0.0f,1.0f}
				});
			m_PolygonDesc.vertices.push_back(
				{
					{finalPt3},
					{0.0f,0.0f,1.0f}
				});
			m_PolygonDesc.indices.push_back(i);
			m_PolygonDesc.indices.push_back(i + 1);

			i += 2;

			auto derPt2U = this->GetSecondDarivativeSameDirection(uParam, vParam, TangentDir::AlongU);
			auto finalPt2U = pt + length * derPt2U;
			m_PolygonDesc.vertices.push_back(
				{
					{pt},
					{0.5f,0.0f,0.0f}
				});
			m_PolygonDesc.vertices.push_back(
				{
					{finalPt2U},
					{0.5f,0.0f,0.0f}
				});
			m_PolygonDesc.indices.push_back(i);
			m_PolygonDesc.indices.push_back(i + 1);

			i += 2;

			auto derPt2V = this->GetSecondDarivativeSameDirection(uParam, vParam, TangentDir::AlongV);
			auto finalPt2V = pt + length * derPt2V;
			m_PolygonDesc.vertices.push_back(
				{
					{pt},
					{0.0f,0.5f,0.0f}
				});
			m_PolygonDesc.vertices.push_back(
				{
					{finalPt2V},
					{0.0f,0.5f,0.0f}
				});
			m_PolygonDesc.indices.push_back(i);
			m_PolygonDesc.indices.push_back(i + 1);

			i += 2;*/
		}
	}


	/*for (int i = 0; i < 16; i++) {
		m_PolygonDesc.vertices.push_back(VertexPositionColor{
			GetReferences().GetAllRef()[i].m_refered.lock()->m_object->GetPosition(),
			m_meshDesc.m_defaultColor });
	}

	for (int i = 0; i < 4; i++) {
		m_PolygonDesc.indices.push_back(4*i);
		m_PolygonDesc.indices.push_back(4*i + 1);
		m_PolygonDesc.indices.push_back(4*i + 1);
		m_PolygonDesc.indices.push_back(4*i + 2);
		m_PolygonDesc.indices.push_back(4*i + 2);
		m_PolygonDesc.indices.push_back(4*i + 3);
	}

	for (int i = 0; i < 4; i++) {
		m_PolygonDesc.indices.push_back(i);
		m_PolygonDesc.indices.push_back(i + 4); 
		m_PolygonDesc.indices.push_back(i + 4);
		m_PolygonDesc.indices.push_back(i + 8);
		m_PolygonDesc.indices.push_back(i + 8);
		m_PolygonDesc.indices.push_back(i + 12);
	}*/


	m_meshDesc.vertices = vertices;
	m_UDesc.vertices = m_VDesc.vertices = vertices;
	m_UDesc.indices = indicesU;
	m_VDesc.indices = indicesV;
	//m_meshDesc.indices = indices;
}

bool BezierPatch::GetIsModified()
{
	bool modified = m_modified;

	auto points = GetReferences().GetAllRef();
	for (int i = 0; i < 16; i++)
	{
		modified |= points[i].m_refered.lock()->m_object->GetIsModified();
	}

	return modified;
}

void BezierPatch::SetDivisions(int divsU, int divsV)
{
	m_uSize = divsU;
	m_vSize = divsV;
}

void BezierPatch::SetPolygonVisible(bool state)
{
	m_displayPolygon = state;
}

void BezierPatch::SetPolygonColor(DirectX::XMFLOAT3 col)
{
	m_PolygonDesc.m_adjustableColor = col;
	SetModified(true);
}

void BezierPatch::SetPatchColor(DirectX::XMFLOAT3 col)
{
	m_meshDesc.m_adjustableColor = col;
	SetModified(true);
}

BezierPatchPointPos BezierPatch::GetPatchPointPositions()
{
	BezierPatchPointPos patch;

	auto refs = GetReferences().GetAllRef();
	DirectX::XMFLOAT3 p00, p01, p02, p03;
	p00 = refs[0].m_refered.lock()->m_object->GetPosition();
	p01 = refs[1].m_refered.lock()->m_object->GetPosition();
	p02 = refs[2].m_refered.lock()->m_object->GetPosition();
	p03 = refs[3].m_refered.lock()->m_object->GetPosition();	
	patch.row0 = BezierCoeffs{ p00,p01,p02,p03 };

	DirectX::XMFLOAT3 p10, p11, p12, p13;
	p10 = refs[4].m_refered.lock()->m_object->GetPosition();
	p11 = refs[5].m_refered.lock()->m_object->GetPosition();
	p12 = refs[6].m_refered.lock()->m_object->GetPosition();
	p13 = refs[7].m_refered.lock()->m_object->GetPosition();
	patch.row1 = BezierCoeffs{ p10,p11,p12,p13 };

	DirectX::XMFLOAT3 p20, p21, p22, p23;
	p20 = refs[8].m_refered.lock()->m_object->GetPosition();
	p21 = refs[9].m_refered.lock()->m_object->GetPosition();
	p22 = refs[10].m_refered.lock()->m_object->GetPosition();
	p23 = refs[11].m_refered.lock()->m_object->GetPosition();
	patch.row2 = BezierCoeffs{ p20,p21,p22,p23 };

	DirectX::XMFLOAT3 p30, p31, p32, p33;
	p30 = refs[12].m_refered.lock()->m_object->GetPosition();
	p31 = refs[13].m_refered.lock()->m_object->GetPosition();
	p32 = refs[14].m_refered.lock()->m_object->GetPosition();
	p33 = refs[15].m_refered.lock()->m_object->GetPosition();	
	patch.row3 = BezierCoeffs{ p30,p31,p32,p33 };

	return patch;
}

std::vector<std::weak_ptr<Node>> BezierPatch::GetPoints(BoundaryDirection direction)
{
	std::vector<std::weak_ptr<Node>> points;
	auto refs = GetReferences().GetAllRef();
	switch (direction) {
	case BoundaryDirection::Top:
		points.push_back(refs[0].m_refered);
		points.push_back(refs[1].m_refered);
		points.push_back(refs[2].m_refered);
		points.push_back(refs[3].m_refered);
		break;
	case BoundaryDirection::Bottom:
		points.push_back(refs[12].m_refered);
		points.push_back(refs[13].m_refered);
		points.push_back(refs[14].m_refered);
		points.push_back(refs[15].m_refered);
		break;
	case BoundaryDirection::Left:
		points.push_back(refs[0].m_refered);
		points.push_back(refs[4].m_refered);
		points.push_back(refs[8].m_refered);
		points.push_back(refs[12].m_refered);
		break;
	case BoundaryDirection::Right:
		points.push_back(refs[3].m_refered);
		points.push_back(refs[7].m_refered);
		points.push_back(refs[11].m_refered);
		points.push_back(refs[15].m_refered);
		break;
	}

	return points;
}

std::vector<std::weak_ptr<Node>> BezierPatch::GetPreBoundaryPoints(BoundaryDirection direction)
{
	std::vector<std::weak_ptr<Node>> points;
	auto refs = GetReferences().GetAllRef();
	switch (direction) {
	case BoundaryDirection::Top:
		points.push_back(refs[4].m_refered);
		points.push_back(refs[5].m_refered);
		points.push_back(refs[6].m_refered);
		points.push_back(refs[7].m_refered);
		break;
	case BoundaryDirection::Bottom:
		points.push_back(refs[8].m_refered);
		points.push_back(refs[9].m_refered);
		points.push_back(refs[10].m_refered);
		points.push_back(refs[1].m_refered);
		break;
	case BoundaryDirection::Left:
		points.push_back(refs[1].m_refered);
		points.push_back(refs[5].m_refered);
		points.push_back(refs[9].m_refered);
		points.push_back(refs[13].m_refered);
		break;
	case BoundaryDirection::Right:
		points.push_back(refs[2].m_refered);
		points.push_back(refs[6].m_refered);
		points.push_back(refs[10].m_refered);
		points.push_back(refs[14].m_refered);
		break;
	}

	return points;
}

std::vector<std::weak_ptr<Node>> BezierPatch::GetPoints(RowPlace row)
{
	std::vector<std::weak_ptr<Node>> points;
	auto refs = GetReferences().GetAllRef();
	switch (row) {
	case RowPlace::First:
		points.push_back(refs[0].m_refered);
		points.push_back(refs[1].m_refered);
		points.push_back(refs[2].m_refered);
		points.push_back(refs[3].m_refered);
		break;
	case RowPlace::Second:
		points.push_back(refs[4].m_refered);
		points.push_back(refs[5].m_refered);
		points.push_back(refs[6].m_refered);
		points.push_back(refs[7].m_refered);		
		break;
	case RowPlace::Third:
		points.push_back(refs[8].m_refered);
		points.push_back(refs[9].m_refered);
		points.push_back(refs[10].m_refered);
		points.push_back(refs[11].m_refered);
		break;
	case RowPlace::Fourth:
		points.push_back(refs[12].m_refered);
		points.push_back(refs[13].m_refered);
		points.push_back(refs[14].m_refered);
		points.push_back(refs[15].m_refered);
		break;
	}

	return points;
}

float GetCoord(std::weak_ptr<Node> point, Coord coord)
{
	if (auto pt = point.lock())
	{
		auto pos = pt->m_object->GetPosition();
		switch (coord) {
		case Coord::Xpos:
			return pos.x;
		case Coord::Ypos:
return pos.y;
		case Coord::Zpos:
			return pos.z;
		}
	}

	return NAN;
}


XMMATRIX BezierPatch::GetCoordinates(Coord coord)
{

	auto refs = GetReferences().GetAllRef();
	XMFLOAT4X4 mat{
		GetCoord(refs[0].m_refered, coord), GetCoord(refs[1].m_refered, coord), GetCoord(refs[2].m_refered, coord), GetCoord(refs[3].m_refered, coord),
		GetCoord(refs[4].m_refered, coord), GetCoord(refs[5].m_refered, coord), GetCoord(refs[6].m_refered, coord), GetCoord(refs[7].m_refered, coord),
		GetCoord(refs[8].m_refered, coord), GetCoord(refs[9].m_refered, coord), GetCoord(refs[10].m_refered, coord), GetCoord(refs[11].m_refered, coord),
		GetCoord(refs[12].m_refered, coord), GetCoord(refs[13].m_refered, coord), GetCoord(refs[14].m_refered, coord), GetCoord(refs[15].m_refered, coord)
	};
	
	return XMLoadFloat4x4(&mat);
}

void BezierPatch::RenderObjectSpecificContextOptions(Scene& scene)
{
	if (ImGui::Button("Place debug points"))
	{
		for (int u = 0; u <= 10; u++)
		{
			for (int v = 0; v <= 10; v++)
			{
				float paramV = 1.f / 10.f * (float)v;
				float paramU = 1.f / 10.f * (float)u;

				auto pos = this->GetPoint(paramU, paramV);
				Transform t;
				t.SetPosition(pos);
				auto pt = scene.m_objectFactory->CreatePoint(t);
				scene.AttachObject(pt);
			}
		}
	}
}

ParameterPair BezierPatch::GetMaxParameterValues()
{
	ParameterPair res;
	res.u = 1.0f;
	res.v = 1.0f;
	return res;
}

ParameterPair BezierPatch::GetNormalizedParams(float u, float v)
{
	// Intentional - no need to scale
	return ParameterPair(u,v);
}

DirectX::XMFLOAT3 BezierPatch::GetPoint(float u, float v)
{
	auto refs = GetReferences().GetAllRef();

	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;

	auto points = GetPatchPointPositions();
	
	aux0 = BezierCalculator::CalculateBezier4(points.row0, u);
	aux1 = BezierCalculator::CalculateBezier4(points.row1, u);	
	aux2 = BezierCalculator::CalculateBezier4(points.row2, u);	
	aux3 = BezierCalculator::CalculateBezier4(points.row3, u);
	auto res = BezierCalculator::CalculateBezier4(aux0, aux1, aux2, aux3, v);
	return res;
}

DirectX::XMFLOAT3 BezierPatch::GetTangent(float u, float v, TangentDir tangentDir)
{
	auto refs = GetReferences().GetAllRef();
	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;

	auto points = GetPatchPointPositions();

	DirectX::XMFLOAT3 result;

	if (tangentDir == TangentDir::AlongU)
	{
		// Along U
		auto row0Der = BezierCalculator::GetDerivativeCoefficients(points.row0);
		auto row1Der = BezierCalculator::GetDerivativeCoefficients(points.row1);
		auto row2Der = BezierCalculator::GetDerivativeCoefficients(points.row2);
		auto row3Der = BezierCalculator::GetDerivativeCoefficients(points.row3);
		
		aux0 = BezierCalculator::CalculateBezier3(row0Der, u);
		aux1 = BezierCalculator::CalculateBezier3(row1Der, u);
		aux2 = BezierCalculator::CalculateBezier3(row2Der, u);
		aux3 = BezierCalculator::CalculateBezier3(row3Der, u);
		
		result = BezierCalculator::CalculateBezier4(aux0, aux1, aux2, aux3, v);
	}
	else {
		// Along V
		aux0 = BezierCalculator::CalculateBezier4(points.row0, u);
		aux1 = BezierCalculator::CalculateBezier4(points.row1, u);
		aux2 = BezierCalculator::CalculateBezier4(points.row2, u);
		aux3 = BezierCalculator::CalculateBezier4(points.row3, u);
		
		// Calculate diff along V
		auto derPoints = BezierCalculator::GetDerivativeCoefficients(aux0, aux1, aux2, aux3);
		result = BezierCalculator::CalculateBezier3(derPoints,v);
	}

	return result;
}

DirectX::XMFLOAT3 BezierPatch::GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir)
{
	DirectX::XMFLOAT3 result;
	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;
	auto points = GetPatchPointPositions();

	switch (tangentDir)
	{
	case TangentDir::AlongU:

		// Get 4 second derivatives in u direction
		auto coeffs0 = BezierCalculator::GetSecondDerivativeCoefficients(points.row0);
		auto coeffs1 = BezierCalculator::GetSecondDerivativeCoefficients(points.row1);
		auto coeffs2 = BezierCalculator::GetSecondDerivativeCoefficients(points.row2);
		auto coeffs3 = BezierCalculator::GetSecondDerivativeCoefficients(points.row3);

		aux0 = BezierCalculator::CalculateBezier2(coeffs0.b210, coeffs0.b321, u);
		aux1 = BezierCalculator::CalculateBezier2(coeffs1.b210, coeffs1.b321, u);
		aux2 = BezierCalculator::CalculateBezier2(coeffs2.b210, coeffs2.b321, u);
		aux3 = BezierCalculator::CalculateBezier2(coeffs3.b210, coeffs3.b321, u);

		// Calculate value from 4 vectors
		result = BezierCalculator::CalculateBezier4(aux0, aux1, aux2, aux3, v);
		break;
	case TangentDir::AlongV:
		// calculate 4 points in u direction
		aux0 = BezierCalculator::CalculateBezier4(points.row0, u);
		aux1 = BezierCalculator::CalculateBezier4(points.row1, u);
		aux2 = BezierCalculator::CalculateBezier4(points.row2, u);
		aux3 = BezierCalculator::CalculateBezier4(points.row3, u);
		// calculate second derivative in the v direction based on 4 points
		auto coeffs = BezierCalculator::GetSecondDerivativeCoefficients(aux0, aux1, aux2, aux3);
		result = BezierCalculator::CalculateBezier2(coeffs.b210, coeffs.b321, v);
		break;
	}

	return result;
}

DirectX::XMFLOAT3 BezierPatch::GetSecondDarivativeMixed(float u, float v)
{
	auto refs = GetReferences().GetAllRef();
	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;

	auto points = GetPatchPointPositions();

	// Differentiate in  the u direction
	auto uDer0 = BezierCalculator::GetDerivativeCoefficients(points.row0);
	auto uDer1 = BezierCalculator::GetDerivativeCoefficients(points.row1);
	auto uDer2 = BezierCalculator::GetDerivativeCoefficients(points.row2);
	auto uDer3 = BezierCalculator::GetDerivativeCoefficients(points.row3);

	// Differentiate in  the v direction
	auto uvDer0 = BezierCalculator::GetDerivativeCoefficients(uDer0.b10, uDer1.b10, uDer2.b10, uDer3.b10);
	auto uvDer1 = BezierCalculator::GetDerivativeCoefficients(uDer0.b21, uDer1.b21, uDer2.b21, uDer3.b21);
	auto uvDer2 = BezierCalculator::GetDerivativeCoefficients(uDer0.b32, uDer1.b32, uDer2.b32, uDer3.b32);

	// Calculate one side of the Bezier tensor 
	auto finalCoeff0 = BezierCalculator::CalculateBezier3(uvDer0, v);
	auto finalCoeff1 = BezierCalculator::CalculateBezier3(uvDer1, v);
	auto finalCoeff2 = BezierCalculator::CalculateBezier3(uvDer2, v);

	// Calculate the other side of the Bezier tensor
	return BezierCalculator::CalculateBezier3(finalCoeff0, finalCoeff1, finalCoeff2, u);
}

bool BezierPatch::ParamsInsideBounds(float u, float v)
{
	bool UinRange = u >= 0 && u <= 1.f;
	bool VinRange = v >= 0 && v <= 1.f;
	return (UinRange && VinRange);
}

void BezierPatch::GetWrappedParams(float& u, float& v)
{
	// nothing to do here
}

float BezierPatch::GetFarthestPointInDirection(float u, float v, DirectX::XMFLOAT2 dir, float defStep)
{
	float res = defStep;
	DirectX::XMFLOAT2 params = DirectX::XMFLOAT2(u, v);
	DirectX::XMFLOAT2 movedParams = params + dir * defStep;

	if (ParamsInsideBounds(movedParams.x, movedParams.y) == false)
	{
		// check for nearest intersection with u and v edges
		// Check u edges
		if (dir.x != 0.0f)
		{
			float step0 = (0.f - u) / dir.x;
			float step1 = (1.f - u) / dir.x;

			if (step0 * defStep > 0 && abs(step0) < abs(res)) //has the same sign
			{
				res = step0;
			}

			if (step1 * defStep > 0 && abs(step1) < abs(res)) //has the same sign
			{
				res = step1;
			}
		}

		// Check v edges
		if (dir.y != 0.0f)
		{
			float step0 = (0.f - v) / dir.y;
			float step1 = (1.f - v) / dir.y;

			if (step0 * defStep > 0 && abs(step0) < abs(res)) //has the same sign
			{
				res = step0;
			}

			if (step1 * defStep > 0 && abs(step1) < abs(res)) //has the same sign
			{
				res = step1;
			}
		}

	}

	return res;
}
