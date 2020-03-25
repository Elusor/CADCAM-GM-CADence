#pragma once
#include "Object.h"

struct Point : Object
{
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
	void RenderObjectSpecificContextOptions(Scene& scene);
};
