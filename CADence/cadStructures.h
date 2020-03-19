#pragma once
#include <vector>
#include "Transform.h"
#include "mathStructures.h"
#include "renderData.h"
#include <string>
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

	std::string m_name = "";
	std::string m_defaultName = "";
	Transform m_transform;	
	virtual void RenderObject(std::unique_ptr<RenderData> & renderData, bool selected = false);
	virtual void RenderCoordinates(std::unique_ptr<RenderData>& renderData, bool selected = false );
	virtual void UpdateObject();
	virtual bool CreateParamsGui();
};

struct Point : Object
{
	void RenderObject(std::unique_ptr<RenderData>& renderData, bool selected = false) override;
	bool CreateParamsGui() override;
};

struct SpawnMarker : Object 
{
	void RenderObject(std::unique_ptr<RenderData>& renderData, bool selected = false) override;
	bool CreateParamsGui() override;
};

struct SurfaceObject : Object
{
	SurfaceParametrizationParams m_surParams;
	SurfaceVerticesDescription m_surVerDesc;

	bool CreateParamsGui() override;
	void RenderObject(std::unique_ptr<RenderData> & renderData, bool selected = false) override;
};

struct Torus : SurfaceObject 
{
	float m_bigR;
	float m_smallR;

	bool CreateParamsGui() override;
	void UpdateObject() override;
};