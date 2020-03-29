#pragma once
#include "BezierCurve.h"

enum BezierBasis
{
	Bernstein,
	BSpline
};

struct BezierCurveC2 : BezierCurve
{
public:
	BezierCurveC2();
	explicit BezierCurveC2(std::vector<std::weak_ptr<Node>> initialControlPoints, BezierBasis basis = BezierBasis::BSpline);
	
	void UpdateObject() override;
	bool CreateParamsGui() override;
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	void AttachChild(std::weak_ptr<Node> controlPoint) override;
	void RemoveChild(std::weak_ptr<Node> controlPoint) override;
	void RecalculateBasisPoints(bool overwriteVertices = true);

private:
	// Basis in which the curve is represented
	BezierBasis m_basis;
	// Data concerning the De Boor polygon
	bool m_renderDeBoorPolygon;
	MeshDescription m_deBoorPolyDesc;

	// Holds nodes displayed in the group node
	std::vector<std::shared_ptr<Node>> m_curBasisControlPoints;
	// Holds the positions for Bernstein points calculated from DeBoore
	std::vector<DirectX::XMFLOAT3> m_virtualBernsteinPoints;	
		
	void SwitchBases();
	void RecalculateBSplinePoints(bool overwriteVertices = true);
	void RecalculateBernsteinPoints(bool overwriteVertices = true);
	std::vector<DirectX::XMFLOAT3> CalculateBernsteinFromDeBoor();
};