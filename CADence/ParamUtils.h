#pragma once
#include <DirectXMath.h>
struct ParameterPair
{
	float u;
	float v;

	ParameterPair() {};
	ParameterPair(DirectX::XMFLOAT2 uv) : u{ uv.x }, v{ uv.y} {};
	ParameterPair(float _u, float _v) : u{ _u }, v{ _v} {};
	
	DirectX::XMFLOAT2 GetVector();

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
ParameterPair operator+(ParameterPair a, ParameterPair b);