#pragma once
#include "Renderer.h"
#include "cadStructures.h"

class GizmoRenderer: Renderer
{
public:
	void Render() override;
	void RenderGizmo(Object* object);
};