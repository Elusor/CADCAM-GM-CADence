#pragma once
#include "Object.h"
#include "Point.h"

class Node; 
class Scene;

struct BezierCurve : Object
{
public:
	// TODO MG maybe just change objects to sharedptr?
	explicit BezierCurve(std::vector<std::weak_ptr<Node>> initialControlPoints);

	std::vector<std::weak_ptr<Node>> m_controlPoints;
	
	void AttachChild(std::weak_ptr<Node> controlPoint);
	void RemoveChild(std::weak_ptr<Node> controlPoint);
	//void DetachPoint(std::weak_ptr<Point> controlPoint);
	bool IsChild(std::weak_ptr<Node> point);
	void RenderObjectSpecificContextOptions(Scene& scene) override;
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	void UpdateObject() override;
	bool CreateParamsGui() override;

private:
	bool m_renderPolygon;

};