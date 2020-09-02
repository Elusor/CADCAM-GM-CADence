#pragma once
#include <DirectXMath.h>
struct ParameterPair
{
	ParameterPair() {};
	ParameterPair(float _u, float _v) : u{ _u }, v{ _v} {};
	float u;
	float v;
};

struct ParameterQuad
{
	float u;
	float v;
	float s;
	float t;

	ParameterQuad();
	ParameterQuad(DirectX::XMFLOAT4 params);

	ParameterPair GetQParams();
	ParameterPair GetPParams();
	DirectX::XMFLOAT4 GetVector();

	void Set(ParameterPair qParams, ParameterPair pParams);

};

ParameterQuad operator+(ParameterQuad a, ParameterQuad b);