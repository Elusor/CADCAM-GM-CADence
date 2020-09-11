#pragma once
#include <vector>
#include <DirectXMath.h>
#include "IParametricSurface.h"
#include "MeshObject.h"
#include "BezierCurve.h"
#include "IntersectedSurface.h"

class IntersectionCurve: public BezierCurve
{
public:
	IntersectionCurve();
	~IntersectionCurve();
	void Initialize(
		ObjectRef qSurface, std::vector<DirectX::XMFLOAT2> qParameters, bool qIsLooped,
		ObjectRef pSurface, std::vector<DirectX::XMFLOAT2> pParameters, bool pIsLooped);

	// Intersection Curve Unique methods
	ObjectRef GetParametricSurface(IntersectedSurface surface);
	bool GetIsClosedIntersection(IntersectedSurface surface);
	std::vector<DirectX::XMFLOAT2> GetParameterList(IntersectedSurface surface);
	std::vector<DirectX::XMFLOAT2> GetNormalizedParameterList(IntersectedSurface surface);	
	std::vector<DirectX::XMFLOAT3> GetPointPositions();

	// Object methods
	virtual void RenderObjectSpecificContextOptions(Scene& scene);

	// Rendering and updates
	void UpdateObject() override;
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
	bool GetIsModified() override;

private:
	DirectX::XMFLOAT4X4 m_changeBasisMtx = {
			1.f, 1.f, 1.f, 1.f,
			0.f,  1.f / 3.f, 2.f / 3.f, 1.f,
			0.f, 0.f, 1.f / 3.f, 1.f,
			0.f, 0.f, 0.f, 1.f };

	ObjectRef m_qSurface;
	ObjectRef m_pSurface;
	std::vector<DirectX::XMFLOAT2> m_qParameters;
	std::vector<DirectX::XMFLOAT2> m_pParameters;
	std::vector<DirectX::XMFLOAT3> m_positions;	

	bool m_qIsLooped = false;
	bool m_pIsLooped = false;

	int m_uPtsOnBorderMax = 0;
	int m_uPtsOnBorderZero = 0;

	int m_vPtsOnBorderMax = 0;
	int m_vPtsOnBorderZero = 0;

	int m_sPtsOnBorderMax = 0;
	int m_sPtsOnBorderZero = 0;

	int m_tPtsOnBorderMax = 0;
	int m_tPtsOnBorderZero = 0;

	std::vector<std::shared_ptr<Node>> m_virtualPoints;
	std::vector<std::weak_ptr<Node>> m_virtualPointsWeak;
	std::shared_ptr<Node> CreateVirtualPoint(DirectX::XMFLOAT3 pos, int ptIdx);
	void GetInterpolationSplineBernsteinPoints();
	void UpdateGSData();
	void TrimAffectedSurfaces();
	void TrimSurface(IntersectedSurface surface);
	void CountPointsOnBorders(ObjectRef qSurface, std::vector<DirectX::XMFLOAT2> qParameters,
		ObjectRef pSurface, std::vector<DirectX::XMFLOAT2> pParameters);
};