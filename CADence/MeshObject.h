#pragma once
#include "Object.h"
#include "cadStructures.h"
#include "MeshRenderer.h"

struct MeshObject : Object
{	
	VerticesDescription m_surVerDesc;
	MeshRenderer m_renderer;

	bool CreateParamsGui() override;
	void RenderObject() override;	
};