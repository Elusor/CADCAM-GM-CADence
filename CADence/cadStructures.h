#pragma once
#include "Transform.h"
#include <vector>
#include "mathStructures.h"
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
	virtual void RenderObject();
	virtual void UpdateObject();
};

struct SurfaceObject : Object
{
	SurfaceParametrizationParams m_surParams;
	SurfaceVerticesDescription m_surVerDesc;
	Transform m_transform;

	virtual bool CreateObjectsImguiSection();
};

struct Torus : SurfaceObject 
{
	float m_bigR;
	float m_smallR;

	bool CreateObjectsImguiSection() override;
	void RenderObject() override;
	void UpdateObject() override;
};