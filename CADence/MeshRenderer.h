#pragma once
#include "Renderer.h"
#include "MeshObject.h"

class MeshRenderer : Renderer
{
public:
	void Render() override;
	void RenderMesh(MeshObject* object);
};