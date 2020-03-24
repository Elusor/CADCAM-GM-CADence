#include "BezierCurve.h"
#include "Node.h"
BezierCurve::BezierCurve(std::vector<std::weak_ptr<Node>> initialControlPoints)
{
	m_renderPolygon = false;
	m_controlPoints = initialControlPoints;
}

void BezierCurve::AttachChild(std::weak_ptr<Node> controlPoint)
{
	m_controlPoints.push_back(controlPoint);
}

void BezierCurve::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	// Render object using De Casteljau algorithm
}

bool BezierCurve::CreateParamsGui()
{
	// checkbox for Bernstein's polygon
	// checkbox for deBoore points

	return false;
}

void BezierCurve::UpdateObject()
{
	// Recalculate adaptive rendering??
}
