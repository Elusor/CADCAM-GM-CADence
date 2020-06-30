#pragma once
#include "ParametrizedObject2D.h"
#include "IParametricSurface.h"
struct Torus : public ParametrizedObject2D, public IParametricSurface
{
public:
	float m_bigR;
	float m_smallR;

	bool CreateParamsGui() override;
	void UpdateObject() override;

	// Inherited via IParametricSurface
	virtual DirectX::XMFLOAT3 GetPoint(float u, float v) override;
	virtual DirectX::XMFLOAT3 GetTangent(float u, float v, TangentDir tangentDir) override;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeSameDirection(float u, float v, TangentDir tangentDir) override;
	virtual DirectX::XMFLOAT3 GetSecondDarivativeMixed(float u, float v) override;
};