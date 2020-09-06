#include "torusGenerator.h"

unsigned int CalculateIndexForVertex(int x, int y, int innerLoopMaxVal);

//probably move density X and density Y to some SurfaceParameters class/struct
void GetTorusVerticesLineList(Torus* torus)
{
	float R = torus->m_donutR;
	float r = torus->m_tubeR;

	Parametrization2DArguments* surfaceParams = &(torus->m_surParams);
	MeshDescription<VertexParameterColor> * surfaceDescription = &(torus->m_meshDesc);

	surfaceDescription->vertices.clear();
	surfaceDescription->indices.clear();
	// Get the points from torus parametrization
	float maxRotMainRadius = 1.0f;
	float maxRotSmallRadius = 1.0f;

	float densityX = (float)surfaceParams->densityX;
	float densityY = (float)surfaceParams->densityY;

	float bigRotStep = maxRotMainRadius / densityX;
	float smallRotStep = maxRotSmallRadius / densityY;

	torus->m_meshDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

	std::vector<VertexParameterColor> vertices;
	
	float colorFrac = 0.85f;

	for (int u = 0; u <= densityX; u++)
	{
		for (int v = 0; v <= densityY; v++)
		{
			float uParam = (float)u / densityX;
			float vParam = (float)v / densityY;

			VertexParameterColor vertex = {
				{uParam, vParam},
				surfaceDescription->m_defaultColor
			};
			surfaceDescription->vertices.push_back(vertex);
		}
	}

	for (int x = 0; x < densityX; x++)
	{
		for (int y = 0; y < densityY; y++)
		{			
			//Add indices representing edges
			surfaceDescription->indices.push_back(CalculateIndexForVertex(x, y, densityY));
			surfaceDescription->indices.push_back(CalculateIndexForVertex(x + 1, y, densityY));

			surfaceDescription->indices.push_back(CalculateIndexForVertex(x, y, densityY));
			surfaceDescription->indices.push_back(CalculateIndexForVertex(x, (y + 1), densityY));
		}
	}
}

unsigned int CalculateIndexForVertex(int x, int y, int maxUcount)
{
	return x * (maxUcount+1) + y;
}

VertexPositionColor CalculateTorusVertex(float bigR, float smallR, float bigRRot, float smallRRot, float colorFrac)
{

	float x = (smallR * cosf(smallRRot) + bigR) * cosf(bigRRot);
	float y = smallR * sinf(smallRRot);
	float z = (smallR * cosf(smallRRot) + bigR) * sinf(bigRRot);
	VertexPositionColor v{
		{x,y,z},
		{colorFrac,colorFrac,colorFrac}
	};
	return v;
}