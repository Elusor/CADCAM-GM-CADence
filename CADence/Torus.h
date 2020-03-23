#pragma once
#include "ParametrizedObject2D.h"

struct Torus : ParametrizedObject2D
{
	float m_bigR;
	float m_smallR;

	bool CreateParamsGui() override;
	void UpdateObject() override;
};