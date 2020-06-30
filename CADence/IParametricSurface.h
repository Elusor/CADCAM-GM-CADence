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
};