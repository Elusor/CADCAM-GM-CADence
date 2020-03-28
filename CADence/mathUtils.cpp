#include "mathUtils.h"

DirectX::XMFLOAT3 XMF3SUM(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2)
{
	return DirectX::XMFLOAT3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

DirectX::XMFLOAT3 XMFloat3TimesFloat(DirectX::XMFLOAT3 vec, float val)
{
	DirectX::XMFLOAT3 res = DirectX::XMFLOAT3(
		vec.x * val,
		vec.y * val,
		vec.z * val);

	return res;
}

DirectX::XMFLOAT3 WeightedXMFloat3Average(DirectX::XMFLOAT3 f1, DirectX::XMFLOAT3 f2, float t)
{
	return XMF3SUM(XMFloat3TimesFloat(f1, t), XMFloat3TimesFloat(f2, 1.0f - t));
}

// t * p0 + (1-t) * p1
Transform WeightedTransformAverage(Transform p0, Transform p1, float t)
{
	return Transform(
		WeightedXMFloat3Average(p0.GetPosition(), p1.GetPosition(), t),
		WeightedXMFloat3Average(p0.GetRotation(), p1.GetRotation(), t),
		WeightedXMFloat3Average(p0.GetScale(), p1.GetScale(), t));
}