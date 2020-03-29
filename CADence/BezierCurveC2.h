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
	void RecalculateBasisPoints();

private:
	BezierBasis m_basis;
	MeshDescription m_deBoorPolyDesc;
	// These are displayed in the group node
	std::vector<std::shared_ptr<Node>> m_curBasisControlPoints;
	std::vector<DirectX::XMFLOAT3> m_virtualBernsteinPoints;	
		
	void SwitchBases();
	void RecalculateBSplinePoints();
	void RecalculateBernsteinPoints();
	std::vector<DirectX::XMFLOAT3> CalculateBernsteinFromDeBoor();
};