#include "ParamUtils.h"

ParameterQuad::ParameterQuad()
{
}

ParameterQuad::ParameterQuad(DirectX::XMFLOAT4 params)
{
	u = params.x;
	v = params.y;
	s = params.z;
	t = params.w;
}

void ParameterQuad::Set(ParameterPair qParams, ParameterPair pParams)
{
	u = qParams.u;
	v = qParams.v;
	s = pParams.u;
	t = pParams.v;
}

ParameterPair ParameterQuad::GetQParams()
{
	ParameterPair Q;
	Q.u = u;
	Q.v = v;

	return Q;
}

ParameterPair ParameterQuad::GetPParams()
{
	ParameterPair P;
	P.u = s;
	P.v = t;

	return P;
}

DirectX::XMFLOAT4 ParameterQuad::GetVector()
{
	DirectX::XMFLOAT4 res;

	res.x = u;
	res.y = v;
	res.z = s;
	res.w = t;

	return res;
}

ParameterQuad operator+(ParameterQuad a, ParameterQuad b)
{
	ParameterQuad res;

	res.u = a.u + b.u;
	res.v = a.v + b.v;
	res.s = a.s + b.s;
	res.t = a.t + b.t;

	return res;
}

