#include "BezierPatch.h"
#include "imgui.h"
#include "bezierCalculator.h"
#include "mathUtils.h"
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

	divs = XMFLOAT4(m_vSize, 0.0f, 0.f, 0.f);
	divBuff = renderState->SetConstantBuffer<XMFLOAT4>(renderState->m_cbPatchDivisions.get(), divs);
	ID3D11Buffer* divCBuffer2[] = { divBuff }; //, VPbuffer
	renderState->m_device.context()->HSSetConstantBuffers(0, 1, divCBuffer2);
	MeshObject::RenderMesh(renderState, m_VDesc);
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

	for (int i = 0; i < 4; i++) {
		indicesV.push_back(i );
		indicesV.push_back(i + 4);
		indicesV.push_back(i + 8);
		indicesV.push_back(i + 12);
	}
	
	m_PolygonDesc.vertices.clear();
	m_PolygonDesc.indices.clear();
	m_PolygonDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

	for (int i = 0; i < 16; i++) {
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
	}


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
	m_PolygonDesc.m_defaultColor = col;
	SetModified(true);
}

void BezierPatch::SetPatchColor(DirectX::XMFLOAT3 col)
{
	m_meshDesc.m_defaultColor = col;
	SetModified(true);
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

DirectX::XMFLOAT3 BezierPatch::GetPoint(float u, float v)
{
	auto refs = GetReferences().GetAllRef();

	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;

	DirectX::XMFLOAT3 p00, p01, p02, p03;
	p00 = refs[0].m_refered.lock()->m_object->GetPosition();
	p01 = refs[1].m_refered.lock()->m_object->GetPosition();
	p02 = refs[2].m_refered.lock()->m_object->GetPosition();
	p03 = refs[3].m_refered.lock()->m_object->GetPosition();
	aux0 = BezierCalculator::CalculateBezier4(p00, p01, p02, p03, u);

	DirectX::XMFLOAT3 p10, p11, p12, p13;
	p10 = refs[4].m_refered.lock()->m_object->GetPosition();
	p11 = refs[5].m_refered.lock()->m_object->GetPosition();
	p12 = refs[6].m_refered.lock()->m_object->GetPosition();
	p13 = refs[7].m_refered.lock()->m_object->GetPosition();
	aux1 = BezierCalculator::CalculateBezier4(p10, p11, p12, p13, u);

	DirectX::XMFLOAT3 p20, p21, p22, p23;
	p20 = refs[8].m_refered.lock()->m_object->GetPosition();
	p21 = refs[9].m_refered.lock()->m_object->GetPosition();
	p22 = refs[10].m_refered.lock()->m_object->GetPosition();
	p23 = refs[11].m_refered.lock()->m_object->GetPosition();
	aux2 = BezierCalculator::CalculateBezier4(p20, p21, p22, p23, u);

	DirectX::XMFLOAT3 p30, p31, p32, p33;
	p30 = refs[12].m_refered.lock()->m_object->GetPosition();
	p31 = refs[13].m_refered.lock()->m_object->GetPosition();
	p32 = refs[14].m_refered.lock()->m_object->GetPosition();
	p33 = refs[15].m_refered.lock()->m_object->GetPosition();
	aux3 = BezierCalculator::CalculateBezier4(p30, p31, p32, p33, u);

	return BezierCalculator::CalculateBezier4(aux0, aux1, aux2, aux3, v);
}

DirectX::XMFLOAT3 BezierPatch::GetTangent(float u, float v, TangentDir tangentDir)
{
	auto refs = GetReferences().GetAllRef();
	DirectX::XMFLOAT3 aux0, aux1, aux2, aux3;

	DirectX::XMFLOAT3 p00, p01, p02, p03;
	p00 = refs[0].m_refered.lock()->m_object->GetPosition();
	p01 = refs[1].m_refered.lock()->m_object->GetPosition();
	p02 = refs[2].m_refered.lock()->m_object->GetPosition();
	p03 = refs[3].m_refered.lock()->m_object->GetPosition();

	DirectX::XMFLOAT3 p10, p11, p12, p13;
	p10 = refs[4].m_refered.lock()->m_object->GetPosition();
	p11 = refs[5].m_refered.lock()->m_object->GetPosition();
	p12 = refs[6].m_refered.lock()->m_object->GetPosition();
	p13 = refs[7].m_refered.lock()->m_object->GetPosition();

	DirectX::XMFLOAT3 p20, p21, p22, p23;
	p20 = refs[8].m_refered.lock()->m_object->GetPosition();
	p21 = refs[9].m_refered.lock()->m_object->GetPosition();
	p22 = refs[10].m_refered.lock()->m_object->GetPosition();
	p23 = refs[11].m_refered.lock()->m_object->GetPosition();

	DirectX::XMFLOAT3 p30, p31, p32, p33;
	p30 = refs[12].m_refered.lock()->m_object->GetPosition();
	p31 = refs[13].m_refered.lock()->m_object->GetPosition();
	p32 = refs[14].m_refered.lock()->m_object->GetPosition();
	p33 = refs[15].m_refered.lock()->m_object->GetPosition();

	DirectX::XMFLOAT3 result;

	if (tangentDir == TangentDir::AlongU)
	{
		// Along U
		auto aux00 = BezierCalculator::CalculateBezier3(p00, p01, p02, u);
		auto aux01 = BezierCalculator::CalculateBezier3(p01, p02, p03, u);
		aux0 = aux01 - aux00;
		
		auto aux10 = BezierCalculator::CalculateBezier3(p10, p11, p12, u);
		auto aux11 = BezierCalculator::CalculateBezier3(p11, p12, p13, u);
		aux1 = aux11 - aux10;
		
		auto aux20 = BezierCalculator::CalculateBezier3(p20, p21, p22, u);
		auto aux21 = BezierCalculator::CalculateBezier3(p21, p22, p23, u);
		aux2 = aux21 - aux20;

		auto aux30 = BezierCalculator::CalculateBezier3(p30, p31, p32, u);
		auto aux31 = BezierCalculator::CalculateBezier3(p31, p32, p33, u);
		aux3 = aux31 - aux30;

		result = BezierCalculator::CalculateBezier4(aux0, aux1, aux2, aux3, v);
	}
	else {
		// Along V
		aux0 = BezierCalculator::CalculateBezier4(p00, p01, p02, p03, u);
		aux1 = BezierCalculator::CalculateBezier4(p10, p11, p12, p13, u);
		aux2 = BezierCalculator::CalculateBezier4(p20, p21, p22, p23, u);
		aux3 = BezierCalculator::CalculateBezier4(p30, p31, p32, p33, u);
		
		// Calculate diff along V
		auto p0 = BezierCalculator::CalculateBezier3(aux0, aux1, aux2, v);
		auto p1 = BezierCalculator::CalculateBezier3(aux1, aux2, aux3, v);
		result = p1 - p0;
	}

	return result;
}
