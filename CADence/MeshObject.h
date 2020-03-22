#pragma once
#include "Object.h"
#include "objects.h"
#include "MeshRenderer.h"

struct VerticesDescription
{
	D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology;
	std::vector<VertexPositionColor> vertices;
	std::vector<unsigned short> indices;
};

struct MeshObject : Object
{	
	VerticesDescription m_surVerDesc;
	MeshRenderer m_renderer;

	bool CreateParamsGui() override;
	void RenderObject() override;	
};