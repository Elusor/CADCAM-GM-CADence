#include "BezierPatch.h"

BezierPatch::BezierPatch()
{
}

BezierPatch::~BezierPatch()
{
	for (int i = 0; i < 4; i++)
	{
		if (auto pt = m_u0[i].lock()) {
			pt->m_object->RefRelease();
		}
		if (auto pt = m_u1[i].lock()) {
			pt->m_object->RefRelease();
		}
		if (auto pt = m_u2[i].lock()) {
			pt->m_object->RefRelease();
		}
		if (auto pt = m_u3[i].lock()) {
			pt->m_object->RefRelease();
		}
	}
}

BezierPatch::BezierPatch(
	std::vector<std::weak_ptr<Node>> top,
	std::vector<std::weak_ptr<Node>> bottom,
	std::vector<std::weak_ptr<Node>> left,
	std::vector<std::weak_ptr<Node>> right,
	std::vector<std::weak_ptr<Node>> inner)
{	
	m_displayPolygon = false;
	SetPoints(BoundaryDirection::Top, top);	
	SetPoints(BoundaryDirection::Bottom, bottom);	
	SetPoints(BoundaryDirection::Left, left);	
	SetPoints(BoundaryDirection::Right, right);
	UpdateObject();
}

BezierPatch::BezierPatch(
	std::vector<std::weak_ptr<Node>> first, 
	std::vector<std::weak_ptr<Node>> second, 
	std::vector<std::weak_ptr<Node>> third, 
	std::vector<std::weak_ptr<Node>> fourth)
{
	m_displayPolygon = false;
	SetPoints(RowPlace::First, first);
	SetPoints(RowPlace::Second, second);
	SetPoints(RowPlace::Third, third);
	SetPoints(RowPlace::Fourth, fourth);
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
	for (int i = 0; i < 4; i++) {
		vertices.push_back(VertexPositionColor{
			m_u0[i].lock()->m_object->GetPosition(),
			m_meshDesc.m_defaultColor });
		indicesU.push_back(i);
	}

	for (int i = 0; i < 4; i++) {
		vertices.push_back(VertexPositionColor{
			m_u1[i].lock()->m_object->GetPosition(),
			m_meshDesc.m_defaultColor });
		indicesU.push_back(i+4);
	}

	for (int i = 0; i < 4; i++) {
		vertices.push_back(VertexPositionColor{
			m_u2[i].lock()->m_object->GetPosition(),
			m_meshDesc.m_defaultColor });
		indicesU.push_back(i+8);
	}

	for (int i = 0; i < 4; i++) {
		vertices.push_back(VertexPositionColor{
			m_u3[i].lock()->m_object->GetPosition(),
			m_meshDesc.m_defaultColor });
		indicesU.push_back(i+12);
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
	for (int i = 0; i < 4; i++) {
		m_PolygonDesc.vertices.push_back(VertexPositionColor{
			m_u0[i].lock()->m_object->GetPosition(),
			m_PolygonDesc.m_defaultColor });
	}

	for (int i = 0; i < 4; i++) {
		m_PolygonDesc.vertices.push_back(VertexPositionColor{
			m_u1[i].lock()->m_object->GetPosition(),
			m_PolygonDesc.m_defaultColor });
	}

	for (int i = 0; i < 4; i++) {
		m_PolygonDesc.vertices.push_back(VertexPositionColor{
			m_u2[i].lock()->m_object->GetPosition(),
			m_PolygonDesc.m_defaultColor });
	}

	for (int i = 0; i < 4; i++) {
		m_PolygonDesc.vertices.push_back(VertexPositionColor{
			m_u3[i].lock()->m_object->GetPosition(),
			m_PolygonDesc.m_defaultColor });
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

	for (int i = 0; i < 4; i++)
	{
		modified |= m_u0[i].lock()->m_object->GetIsModified();
		modified |= m_u1[i].lock()->m_object->GetIsModified();
		modified |= m_u2[i].lock()->m_object->GetIsModified();
		modified |= m_u3[i].lock()->m_object->GetIsModified();
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
	//SetModified(true);
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

void BezierPatch::SetPoints(BoundaryDirection direction, std::vector<std::weak_ptr<Node>> points)
{
	assert(points.size() == 4);
	switch (direction) {
	case BoundaryDirection::Top:
		m_u0[0] = points[0];
		m_u0[1] = points[1];
		m_u0[2] = points[2];
		m_u0[3] = points[3];
		break;
	case BoundaryDirection::Bottom:
		m_u3[0] = points[0];
		m_u3[1] = points[1];
		m_u3[2] = points[2];
		m_u3[3] = points[3];
		break;
	case BoundaryDirection::Left:
		m_u0[0] = points[0];
		m_u1[0] = points[1];
		m_u2[0] = points[2];
		m_u3[0] = points[3];
		break;
	case BoundaryDirection::Right:
		m_u0[3] = points[0];
		m_u1[3] = points[1];
		m_u2[3] = points[2];
		m_u3[3] = points[3];
		break;
	}
}

void BezierPatch::SetPoints(RowPlace row, std::vector<std::weak_ptr<Node>> points)
{
	assert(points.size() == 4);

	for (int i = 0; i < 4; i++)
	{
		points[i].lock()->m_object->RefUse();
	}

	switch (row) {
	case RowPlace::First:
		m_u0[0] = points[0];
		m_u0[1] = points[1];
		m_u0[2] = points[2];
		m_u0[3] = points[3];
		break;
	case RowPlace::Second:
		m_u1[0] = points[0];
		m_u1[1] = points[1];
		m_u1[2] = points[2];
		m_u1[3] = points[3];
		break;
	case RowPlace::Third:
		m_u2[0] = points[0];
		m_u2[1] = points[1];
		m_u2[2] = points[2];
		m_u2[3] = points[3];
		break;
	case RowPlace::Fourth:
		m_u3[0] = points[0];
		m_u3[1] = points[1];
		m_u3[2] = points[2];
		m_u3[3] = points[3];
		break;
	}
}



std::vector<std::weak_ptr<Node>> BezierPatch::GetPoints(BoundaryDirection direction)
{
	std::vector<std::weak_ptr<Node>> points;
	switch (direction) {
	case BoundaryDirection::Top:
		points.push_back(m_u0[0]);
		points.push_back(m_u0[1]);
		points.push_back(m_u0[2]);
		points.push_back(m_u0[3]);
		break;
	case BoundaryDirection::Bottom:
		points.push_back(m_u3[0]);
		points.push_back(m_u3[1]);
		points.push_back(m_u3[2]);
		points.push_back(m_u3[3]);
		break;
	case BoundaryDirection::Left:
		points.push_back(m_u0[0]);
		points.push_back(m_u1[0]);
		points.push_back(m_u2[0]);
		points.push_back(m_u3[0]);
		break;
	case BoundaryDirection::Right:
		points.push_back(m_u0[3]);
		points.push_back(m_u1[3]);
		points.push_back(m_u2[3]);
		points.push_back(m_u3[3]);
		break;
	}

	return points;
}

std::vector<std::weak_ptr<Node>> BezierPatch::GetPoints(RowPlace row)
{
	std::vector<std::weak_ptr<Node>> points;
	switch (row) {
	case RowPlace::First:
		points.push_back(m_u0[0]);
		points.push_back(m_u0[1]);
		points.push_back(m_u0[2]);
		points.push_back(m_u0[3]);
		break;
	case RowPlace::Second:
		points.push_back(m_u1[0]);
		points.push_back(m_u1[1]);
		points.push_back(m_u1[2]);
		points.push_back(m_u1[3]);
		break;
	case RowPlace::Third:
		points.push_back(m_u2[0]);
		points.push_back(m_u2[1]);
		points.push_back(m_u2[2]);
		points.push_back(m_u2[3]);
		break;
	case RowPlace::Fourth:
		points.push_back(m_u3[0]);
		points.push_back(m_u3[1]);
		points.push_back(m_u3[2]);
		points.push_back(m_u3[3]);
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
	auto ints = GetCoord(m_u0[0], coord);
	XMFLOAT4X4 mat{
		GetCoord(m_u0[0], coord), GetCoord(m_u0[1], coord), GetCoord(m_u0[2], coord), GetCoord(m_u0[3], coord),
		GetCoord(m_u1[0], coord), GetCoord(m_u1[1], coord), GetCoord(m_u1[2], coord), GetCoord(m_u1[3], coord),
		GetCoord(m_u2[0], coord), GetCoord(m_u2[1], coord), GetCoord(m_u2[2], coord), GetCoord(m_u2[3], coord),
		GetCoord(m_u3[0], coord), GetCoord(m_u3[1], coord), GetCoord(m_u3[2], coord), GetCoord(m_u3[3], coord)
	};
	
	return XMLoadFloat4x4(&mat);
}