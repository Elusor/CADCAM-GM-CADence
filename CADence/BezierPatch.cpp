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

XMFLOAT4X4 BezierPatch::GetXCoordinates()
{
	return XMFLOAT4X4();
}

XMFLOAT4X4 BezierPatch::GetYCoordinates()
{
	return XMFLOAT4X4();
}

XMFLOAT4X4 BezierPatch::GetZCoordinates()
{
	return XMFLOAT4X4();
}
