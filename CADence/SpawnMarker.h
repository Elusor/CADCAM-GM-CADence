#pragma once
#include "Object.h"

struct SpawnMarker : Object
{
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
};


