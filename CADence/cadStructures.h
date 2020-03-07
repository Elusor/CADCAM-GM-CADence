#pragma once

struct SurfaceParametrizationParams
{
	int densityX;
	int densityY;
};

struct SurfaceVerticesDescription
{
	std::vector<VertexPositionColor> vertices;
	std::vector<unsigned short> indices;
};

struct SurfaceObject 
{
	SurfaceParametrizationParams m_surParams;
	SurfaceVerticesDescription m_surDesc;
};