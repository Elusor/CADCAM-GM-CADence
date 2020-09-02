#pragma once
#include <vector>
#include <DirectXMath.h>
#include "IParametricSurface.h"
#include "MeshObject.h"
#include "BezierCurve.h"

enum IntersectedSurface
{
	SurfaceQ,
	SurfaceP
};

class IntersectionCurve: public BezierCurve
{
public:
	IntersectionCurve();
	void Initialize(
		IParametricSurface* qSurface, std::vector<DirectX::XMFLOAT2> qParameters,
		IParametricSurface* pSurface, std::vector<DirectX::XMFLOAT2> pParameters);

	// Intersection Curve Unique methods
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

	IParametricSurface* m_qSurface;
	IParametricSurface* m_pSurface;
	std::vector<DirectX::XMFLOAT2> m_qParameters;
	std::vector<DirectX::XMFLOAT2> m_pParameters;
	std::vector<DirectX::XMFLOAT3> m_positions;	

	std::vector<std::shared_ptr<Node>> m_virtualPoints;
	std::vector<std::weak_ptr<Node>> m_virtualPointsWeak;
	std::shared_ptr<Node> CreateVirtualPoint(DirectX::XMFLOAT3 pos, int ptIdx);
	void GetInterpolationSplineBernsteinPoints();
	void UpdateGSData();
};