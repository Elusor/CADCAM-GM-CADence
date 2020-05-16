#include "BezierPatch.h"

BezierPatch::BezierPatch()
{
}

BezierPatch::BezierPatch(
	std::vector<std::weak_ptr<Node>> top,
	std::vector<std::weak_ptr<Node>> bottom,
	std::vector<std::weak_ptr<Node>> left,
	std::vector<std::weak_ptr<Node>> right,
	std::vector<std::weak_ptr<Node>> inner)
{	 	
	SetPoints(BoundaryDirection::Top, top);	
	SetPoints(BoundaryDirection::Bottom, top);	
	SetPoints(BoundaryDirection::Left, top);	
	SetPoints(BoundaryDirection::Right, top);
}

BezierPatch::BezierPatch(
	std::vector<std::weak_ptr<Node>> first, 
	std::vector<std::weak_ptr<Node>> second, 
	std::vector<std::weak_ptr<Node>> third, 
	std::vector<std::weak_ptr<Node>> fourth)
{
	SetPoints(RowPlace::First, first);
	SetPoints(RowPlace::Second, first);
	SetPoints(RowPlace::Third, first);
	SetPoints(RowPlace::Fourth, first);
}

void BezierPatch::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	auto context = renderState->m_device.context().get();
	XMMATRIX x = GetCoordinates(Coord::X);
	XMMATRIX y = GetCoordinates(Coord::Y);
	XMMATRIX z = GetCoordinates(Coord::Z);
	XMMATRIX coords[]{ x, y, z };
	XMMATRIX* cbData = coords;
	
	// Check if this works
	auto mBuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbPatchData.get(), cbData, 3);
	ID3D11Buffer* cb[] = { mBuffer };
	context->GSSetConstantBuffers(0, 1, cb);

	// Set geometry shader
	context->GSSetShader(renderState->m_patchGeometryShader.get(), 0, 0);
	MeshObject::RenderObject(renderState);
	context->GSSetShader(nullptr, nullptr, 0);
}

bool BezierPatch::CreateParamsGui()
{
	bool patchChanged = false;
	patchChanged |= MeshObject::CreateParamsGui();
	int dims[2] = { m_uSize, m_vSize };
	std::string dimDrag = "Grid density" + GetIdentifier();
	patchChanged = ImGui::DragInt2(dimDrag.c_str(), dims, 1.0f, 1, 100);

	if (m_uSize < 1)
		m_uSize = 1;
	if (m_vSize < 1)
		m_vSize = 1;

	return false;
}

void BezierPatch::UpdateObject()
{
	m_meshDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	// pass an (u,v) line to the shader
	// one of those is the constant parameter - the other one is 
	std::vector<VertexPositionColor> vertices;
	std::vector<unsigned short> indices;

	for (int i = 0; i <= m_uSize; i++) {
		float u = (float)i / (float)m_uSize;

		vertices.push_back(VertexPositionColor{
			{u, NAN, NAN},
			m_meshDesc.m_defaultColor });
		indices.push_back(i);
	}

	for (int i = 0; i <= m_vSize; i++) {
		float v = (float)i / (float)m_vSize;
		vertices.push_back(VertexPositionColor{
			{NAN, v, NAN},
			m_meshDesc.m_defaultColor });
		indices.push_back(i + m_uSize);
	}

	m_meshDesc.vertices = vertices;
	m_meshDesc.indices = indices;
}

bool BezierPatch::GetIsModified()
{
	bool modified = false;

	for (int i = 0; i < 4; i++)
	{
		modified |= m_u0[i].lock()->m_object->GetIsModified();
		modified |= m_u1[i].lock()->m_object->GetIsModified();
		modified |= m_u2[i].lock()->m_object->GetIsModified();
		modified |= m_u3[i].lock()->m_object->GetIsModified();
	}

	return modified;
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

float GetCoord(std::weak_ptr<Node> point, Coord coord)
{
	if (auto pt = point.lock())
	{
		auto pos = pt->m_object->GetPosition();
		switch (coord) {
		case Coord::X:
			return pos.x;
		case Coord::Y:
			return pos.y;
		case Coord::Z:
			return pos.z;
		}
	}

	return NAN;
}

XMMATRIX BezierPatch::GetCoordinates(Coord coord)
{
	XMFLOAT4X4 mat{
		GetCoord(m_u0[0], coord), GetCoord(m_u0[1], coord), GetCoord(m_u0[2], coord), GetCoord(m_u0[3], coord),
		GetCoord(m_u1[0], coord), GetCoord(m_u1[1], coord), GetCoord(m_u1[2], coord), GetCoord(m_u1[3], coord),
		GetCoord(m_u2[0], coord), GetCoord(m_u2[1], coord), GetCoord(m_u2[2], coord), GetCoord(m_u2[3], coord),
		GetCoord(m_u3[0], coord), GetCoord(m_u3[1], coord), GetCoord(m_u3[2], coord), GetCoord(m_u3[3], coord)
	};
	
	return XMLoadFloat4x4(&mat);
}