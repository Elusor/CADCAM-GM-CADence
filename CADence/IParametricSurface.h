#pragma once
#include <DirectXMath.h>
#include "ParamUtils.h"
#include "SurfaceWrapDirection.h"

enum TangentDir
{
	AlongU,
	AlongV
};

class IParametricSurface {
public:

	virtual float GetFarthestPointInDirection(float u, float v, DirectX::XMFLOAT2 dir, float defStep) = 0;
	virtual ParameterPair GetMaxParameterValues() = 0;
	virtual ParameterPair GetNormalizedParams(float u, float v) = 0;
	virtual DirectX::XMFLOAT2 GetParameterSpaceDistance(ParameterPair point1, ParameterPair point2) = 0;
	virtual DirectX::XMFLOAT3 GetPoint(float u, float v) = 0;
	virtual DirectX::XMFLOAT3 GetTangent(float u, float v, TangentDir tangentDir) = 0;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir) = 0;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeMixed(float u, float v) = 0;
	virtual ParameterPair GetWrappedParams(float u, float v) = 0;
	virtual bool IsWrappedInDirection(SurfaceWrapDirection wrapDir) = 0;
	virtual bool ParamsInsideBounds(float u, float v) = 0;
	
	// When incrementing parameters in a given direction, returns the maximum step that can be taken not to go out of bounds
	// Get the smallest float with the same sign as DefStep that guarantees that after the step parameters will be in bounds

	DirectX::XMFLOAT3 GetPoint(ParameterPair parameters);
	DirectX::XMFLOAT3 GetTangent(ParameterPair parameters, TangentDir tangentDir);
	bool ParamsInsideBounds(ParameterPair parameters);
};