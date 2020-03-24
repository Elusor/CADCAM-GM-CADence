#include "BezierCurve.h"

BezierCurve::BezierCurve(std::vector<std::weak_ptr<Point>> initialControlPoints)
{
	m_renderPolygon = false;
	m_controlPoints = initialControlPoints;
}

void BezierCurve::AttachChild(std::weak_ptr<Point> controlPoint)
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
