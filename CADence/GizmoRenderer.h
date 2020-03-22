#pragma once
#include "Renderer.h"
#include "objects.h"

class GizmoRenderer: Renderer
{
public:
	void RenderGizmo(Object* object);
};