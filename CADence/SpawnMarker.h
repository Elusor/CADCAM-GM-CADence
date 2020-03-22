#pragma once
#include "Object.h"
#include "GizmoRenderer.h"

struct SpawnMarker : Object
{
	GizmoRenderer m_renderer;

	void RenderObject() override;
	bool CreateParamsGui() override;
};