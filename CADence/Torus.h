#pragma once
#include "SurfaceObject.h"

struct Torus : SurfaceObject
{
	float m_bigR;
	float m_smallR;

	bool CreateParamsGui() override;
	void UpdateObject();
};