#pragma once
#include "BezierCurve.h"
#include "mathUtils.h"
#include "imgui.h"

struct InterpolationBezierCurveC2 : BezierCurve
{
public:
	InterpolationBezierCurveC2();
	explicit InterpolationBezierCurveC2(std::vector<std::weak_ptr<Node>> initialKnots);
	
	// Rendering and updates
	void UpdateObject() override;
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
	bool GetIsModified() override;
	
	// Child managment
	void AttachChild(std::weak_ptr<Node> controlPoint) override;
	void RemoveChild(std::weak_ptr<Node> controlPoint) override;

protected:
	bool RemoveExpiredChildren() override;

private:
	std::vector<std::weak_ptr<Node>> m_displayPoints;

	std::vector<std::shared_ptr<Node>> m_virtualPoints;
	
	// Data concerning the De Boor polygon
	bool m_renderDeBoorPolygon;
	MeshDescription m_deBoorPolyDesc;

	void RecalculateIfModified();
	void GetInterpolationSplineBernsteinPoints(std::vector<Object*> interpolationKnots);

	std::shared_ptr<Node> CreateVirtualPoint(DirectX::XMFLOAT3 pos, int ptIdx);
	void UpdateGSData();
	void PreparePolygonDesc();
};