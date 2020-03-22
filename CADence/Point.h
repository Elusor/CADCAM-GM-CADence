#pragma once
#include "Object.h"
#include "PointRenderer.h"

struct Point : Object
{
	PointRenderer m_renderer;
	void RenderObject() override;
	bool CreateParamsGui() override;
};