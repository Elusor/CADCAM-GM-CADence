#pragma once
#include "ParametrizedObject2D.h"
#include "IParametricSurface.h"
#include "IntersectionData.h"
#include "IntersectableSurface.h"

struct Torus : public ParametrizedObject2D, public IParametricSurface, public TrimmableSurface
{
public:

	float m_donutR;
	float m_tubeR;

	bool CreateParamsGui() override;
	void UpdateObject() override;

	MeshDescription<VertexPositionColor> m_debugDesc;
	
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	void RenderTorus(std::unique_ptr<RenderState>& renderState);	

	// Inherited via IParametricSurface
	virtual ParameterPair GetMaxParameterValues() override;
	virtual ParameterPair GetNormalizedParams(float u, float v) override;
	DirectX::XMFLOAT2 GetParameterSpaceDistance(ParameterPair point1, ParameterPair point2);
	virtual DirectX::XMFLOAT3 GetPoint(float u, float v) override;
	virtual DirectX::XMFLOAT3 GetTangent(float u, float v, TangentDir tangentDir) override;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir) override;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeMixed(float u, float v) override;
	virtual bool IsWrappedInDirection(SurfaceWrapDirection wrapDir) override;
	bool ParamsInsideBounds(float u, float v) override;
	ParameterPair GetWrappedParams(float u, float v) override;
	float GetFarthestPointInDirection(float u, float v, DirectX::XMFLOAT2 dir, float defStep) override;

private:
	
	bool m_displayTangents = false;

};