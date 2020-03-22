#pragma once
#include "Renderer.h"
#include "objects.h"

class MeshRenderer : Renderer
{
public:
	void RenderMesh(MeshObject* object);
};