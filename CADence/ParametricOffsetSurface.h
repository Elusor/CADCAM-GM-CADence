#pragma once
#include "IParametricSurface.h"

class ParametricOffsetSurface : public IParametricSurface
{
public:
	ParametricOffsetSurface(IParametricSurface* parametricSurface, float offsetValue = 0.0f);

	virtual float GetFarthestPointInDirection(float u, float v, DirectX::XMFLOAT2 dir, float defStep) override;
	virtual ParameterPair GetMaxParameterValues() override;
	virtual ParameterPair GetNormalizedParams(float u, float v) override;
	virtual DirectX::XMFLOAT2 GetParameterSpaceDistance(ParameterPair point1, ParameterPair point2) override;
	virtual DirectX::XMFLOAT3 GetPoint(float u, float v) override;
	virtual DirectX::XMFLOAT3 GetTangent(float u, float v, TangentDir tangentDir) override;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir);
	virtual DirectX::XMFLOAT3 GetSecondDarivativeMixed(float u, float v);
	virtual ParameterPair GetWrappedParams(float u, float v) override;
	virtual bool IsWrappedInDirection(SurfaceWrapDirection wrapDir) override;
	virtual bool ParamsInsideBounds(float u, float v) override;

private:
	IParametricSurface* m_baseSurface;
	float m_offsetValue;
};