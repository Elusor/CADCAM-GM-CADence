#include "ParametricOffsetSurface.h"
#include "SimpleMath.h"

ParametricOffsetSurface::ParametricOffsetSurface(IParametricSurface* parametricSurface, float offsetValue)
{
	m_baseSurface = parametricSurface;
	m_offsetValue = offsetValue;
}

float ParametricOffsetSurface::GetFarthestPointInDirection(float u, float v, DirectX::XMFLOAT2 dir, float defStep)
{
	return m_baseSurface->GetFarthestPointInDirection(u, v, dir, defStep);
}

ParameterPair ParametricOffsetSurface::GetMaxParameterValues()
{
	return m_baseSurface->GetMaxParameterValues();
}

ParameterPair ParametricOffsetSurface::GetNormalizedParams(float u, float v)
{
	return m_baseSurface->GetNormalizedParams(u,v);
}

DirectX::XMFLOAT2 ParametricOffsetSurface::GetParameterSpaceDistance(ParameterPair point1, ParameterPair point2)
{
	return m_baseSurface->GetParameterSpaceDistance(point1,point2);
}

DirectX::XMFLOAT3 ParametricOffsetSurface::GetPoint(float u, float v)
{
	// 1. Get base Point
	auto basePoint = m_baseSurface->GetPoint(u, v);
	// 2. Get base normal
	auto normal = DirectX::SimpleMath::Vector3(m_baseSurface->GetNormal(u, v));
	// 3. Offset normal in a given direction
	auto offsetPoint = basePoint + m_offsetValue * normal;

	return offsetPoint;
}

DirectX::XMFLOAT3 ParametricOffsetSurface::GetTangent(float u, float v, TangentDir tangentDir)
{
	float paramEps = 0.0001f;

	float begU = u;
	float endU = u;

	float begV = v;
	float endV = v;

	if (tangentDir == TangentDir::AlongU)
	{
		// Along U
		begU -= paramEps;
		endU += paramEps;
	}
	else
	{
		// Along V
		begV -= paramEps;
		endV += paramEps;
	}

	auto begUV = GetWrappedParams(begU, begV);
	auto endUV = GetWrappedParams(endU, endV);
	
	DirectX::SimpleMath::Vector3 begPoint = GetPoint(begUV.u, begUV.v);
	DirectX::SimpleMath::Vector3 endPoint = GetPoint(endUV.u, endUV.v);

	auto tangent = (endPoint - begPoint) / (2.0f * paramEps);
	return tangent;
}

DirectX::XMFLOAT3 ParametricOffsetSurface::GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir)
{
	return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT3 ParametricOffsetSurface::GetSecondDarivativeMixed(float u, float v)
{
	return DirectX::XMFLOAT3();
}

ParameterPair ParametricOffsetSurface::GetWrappedParams(float u, float v)
{
	return m_baseSurface->GetWrappedParams(u, v);
}

bool ParametricOffsetSurface::IsWrappedInDirection(SurfaceWrapDirection wrapDir)
{
	return m_baseSurface->IsWrappedInDirection(wrapDir);
}

bool ParametricOffsetSurface::ParamsInsideBounds(float u, float v)
{
	return m_baseSurface->ParamsInsideBounds(u, v);
}
