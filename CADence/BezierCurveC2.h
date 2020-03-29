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
	
	void SwitchBases();
	void UpdateObject() override;
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui();
private:

	std::vector<std::shared_ptr<Node>> m_curBasisControlPoints;

	MeshDescription m_deBoorPolyDesc;
	BezierBasis m_basis;
	std::vector<DirectX::XMFLOAT3> m_virtualBernsteinPoints;
	std::vector<DirectX::XMFLOAT3> CalculateBezierFromDeBoor();

};
