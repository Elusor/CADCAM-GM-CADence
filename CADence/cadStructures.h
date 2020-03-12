#pragma once
#include "Transform.h"
#include <vector>
#include "mathStructures.h"
//#include "Scene.h"
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
	std::vector<VertexPositionColor> vertices;
	std::vector<unsigned short> indices;
};

struct Object {

	Transform m_transform;	
	virtual void RenderObject();
	virtual void UpdateObject();
	virtual bool CreateParamsGui();
};

struct SurfaceObject : Object
{
	SurfaceParametrizationParams m_surParams;
	SurfaceVerticesDescription m_surVerDesc;

	bool CreateParamsGui() override;
};

struct Torus : SurfaceObject 
{
	float m_bigR;
	float m_smallR;

	bool CreateParamsGui() override;
	void RenderObject() override;
	void UpdateObject() override;
};