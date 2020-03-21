#include "Renderer.h"
#include "cadStructures.h"

#pragma once
class PointRenderer: Renderer
{
public:
	void Render() override;
	void RenderPoint(Object* object);
};
