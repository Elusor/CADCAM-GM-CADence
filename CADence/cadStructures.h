#pragma once
#include "Transform.h"
#include <vector>
#include "mathStructures.h"
#include "renderData.h"
struct SurfaceParametrizationParams
{
	int densityX;
	int minDensityX;
	int maxDensityX;

	int densityY;
	int minDensityY;
	int maxDensityY;
};

struct SurfaceVerticesDescription
{
	D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology;
	std::vector<VertexPositionColor> vertices;
	std::vector<unsigned short> indices;
};

struct Object {

	Transform m_transform;	
	virtual void RenderObject(RenderData* renderData);
	virtual void UpdateObject();
	virtual bool CreateParamsGui();
};

struct SurfaceObject : Object
{
	SurfaceParametrizationParams m_surParams;
	SurfaceVerticesDescription m_surVerDesc;

	bool CreateParamsGui() override;
	void RenderObject(RenderData* renderData) override;
};

struct Torus : SurfaceObject 
{
	float m_bigR;
	float m_smallR;

	bool CreateParamsGui() override;
	void UpdateObject() override;
};