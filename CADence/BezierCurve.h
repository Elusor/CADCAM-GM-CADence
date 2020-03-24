#pragma once
#include "Object.h"
#include "Point.h"

struct BezierCurve : Object
{
public:
	explicit BezierCurve(std::vector<std::weak_ptr<Point>> initialControlPoints);

	std::vector<std::weak_ptr<Point>> m_controlPoints;
	
	void AttachChild(std::weak_ptr<Point> controlPoint);
	//void DetachPoint(std::weak_ptr<Point> controlPoint);

	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	void UpdateObject() override;
	bool CreateParamsGui() override;

private:
	bool m_renderPolygon;

};