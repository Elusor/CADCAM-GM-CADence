#pragma once
#include <DirectXMath.h>

enum TangentDir
{
	AlongU,
	AlongV
};

__interface IParametricSurface {
public:
	DirectX::XMFLOAT3 GetPoint(float u, float v);
	DirectX::XMFLOAT3 GetTangent(float u, float v, TangentDir tangentDir);
	DirectX::XMFLOAT3 GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir);
	DirectX::XMFLOAT3 GetSecondDarivativeMixed(float u, float v);
	bool ParamsInsideBounds(float u, float v);
	void GetWrappedParams(float& u, float& v);
	// When incrementing parameters in a given direction, returns the maximum step that can be taken not to go out of bounds
	// Get the smallest float with the same sign as DefStep that guarantees that after the step parameters will be in bounds
	float GetFarthestPointInDirection(float u, float v, DirectX::XMFLOAT2 dir, float defStep);
};