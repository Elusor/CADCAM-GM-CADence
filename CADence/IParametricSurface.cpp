#include "IParametricSurface.h"
#include "mathUtils.h"
#include "SimpleMath.h"

DirectX::XMFLOAT3 IParametricSurface::GetPoint(ParameterPair parameters)
{
	return GetPoint(parameters.u, parameters.v);
}

DirectX::XMFLOAT3 IParametricSurface::GetTangent(ParameterPair parameters, TangentDir tangentDir)
{
	return GetTangent(parameters.u, parameters.v, tangentDir);
}

DirectX::XMFLOAT3 IParametricSurface::GetNormal(ParameterPair parameters)
{
	return GetNormal(parameters.u, parameters.v);
}

bool IParametricSurface::ParamsInsideBounds(ParameterPair parameters)
{
	return ParamsInsideBounds(parameters.u, parameters.v);
}

DirectX::XMFLOAT3 IParametricSurface::GetNormal(float u, float v)
{
	DirectX::XMFLOAT3 tangent = GetTangent(u, v, TangentDir::AlongU);
	DirectX::XMFLOAT3 bitangent = GetTangent(u, v, TangentDir::AlongV);
	
	auto res = DirectX::SimpleMath::Vector3(Cross(tangent, bitangent));
	res.Normalize();
	return res;
}
