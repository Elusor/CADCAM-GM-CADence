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
	void Initialize(std::vector<std::weak_ptr<Node>> initialControlPoints, BezierBasis basis = BezierBasis::BSpline);

	void UpdateObject() override;
	bool CreateParamsGui() override;
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	void AttachChild(std::weak_ptr<Node> controlPoint) override;
	void RemoveChild(std::weak_ptr<Node> controlPoint) override;
	void RecalculateBasisPoints(bool overwriteVertices = true);	
	std::vector<std::weak_ptr<Node>> GetDisplayChildren() override;

	bool GetIsModified() override;
	void SetDisplayDeBoorPolygon(bool isDisplayed);
	bool GetDisplayDeBoorPolygon();
	BezierBasis GetCurrentBasis();
protected:
	bool RemoveExpiredChildren() override;

private:
	// Basis in which the curve is represented
	BezierBasis m_basis;
	// Data concerning the De Boor polygon
	bool m_renderDeBoorPolygon;
	MeshDescription m_deBoorPolyDesc;

	// Holds nodes displayed in the group node
	std::vector<std::weak_ptr<Node>> m_curBasisControlPoints;
	std::vector<std::shared_ptr<Node>> m_virtualBernsteinPoints;
	// Holds the positions for Bernstein points calculated from DeBoor
	std::vector<DirectX::XMFLOAT3> m_virtualBernsteinPos;	
	void SwitchBases();
	void RecalculateBSplinePoints(bool overwriteVertices = true);
	void RecalculateBernsteinPoints(bool overwriteVertices = true);
	std::vector<DirectX::XMFLOAT3> CalculateBernsteinFromDeBoor();

	void MoveBernsteinPoint(int index);

	void UpdateGSData();
	void PreparePolygonDesc();
	void RecalculateIfModified();
	void RenderCurve(std::unique_ptr<RenderState>& renderState);
	void RenderPolygon(std::unique_ptr<RenderState>& renderState);
};