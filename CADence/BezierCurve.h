#pragma once
#include "Point.h"
#include "MeshObject.h"
class Node; 
class Scene;

struct BezierCurve : MeshObject
{
public:
	// TODO MG maybe just change objects to sharedptr?
	explicit BezierCurve(std::vector<std::weak_ptr<Node>> initialControlPoints);

	std::vector<std::weak_ptr<Node>> m_controlPoints;
	
	void AttachChild(std::weak_ptr<Node> controlPoint);
	void RemoveChild(std::weak_ptr<Node> controlPoint);
	bool IsChild(std::weak_ptr<Node> point);
	void RenderObjectSpecificContextOptions(Scene& scene) override;
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	void UpdateObject() override;
	bool CreateParamsGui() override;

private:
	bool m_renderPolygon;

};