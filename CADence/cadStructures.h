#pragma once
#include <vector>
#include "Transform.h"
#include "mathStructures.h"
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

struct VerticesDescription
{
	D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology;
	std::vector<VertexPositionColor> vertices;
	std::vector<unsigned short> indices;
};