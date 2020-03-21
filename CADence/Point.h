#pragma once
#include "Object.h"
#include "PointRenderer.h"

struct Point : Object
{
	void RenderObject() override;
	bool CreateParamsGui() override;
};