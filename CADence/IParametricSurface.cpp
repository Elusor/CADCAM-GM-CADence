#include "IParametricSurface.h"

DirectX::XMFLOAT3 IParametricSurface::GetPoint(ParameterPair parameters)
{
	return GetPoint(parameters.u, parameters.v);
}

DirectX::XMFLOAT3 IParametricSurface::GetTangent(ParameterPair parameters, TangentDir tangentDir)
{
	return GetTangent(parameters.u, parameters.v, tangentDir);
}

bool IParametricSurface::ParamsInsideBounds(ParameterPair parameters)
{
	return ParamsInsideBounds(parameters.u, parameters.v);
}
