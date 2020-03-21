#pragma once
#include "Object.h"
#include "GizmoRenderer.h"

struct SpawnMarker : Object
{
	void RenderObject() override;
	bool CreateParamsGui() override;
};