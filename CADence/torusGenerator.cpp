#include "torusGenerator.h"

unsigned int CalculateIndexForVertex(int x, int y, int innerLoopMaxVal);

//probably move density X and density Y to some SurfaceParameters class/struct
void GetTorusVerticesLineList(Torus* torus)
{
	float R = torus->m_bigR;
	float r = torus->m_smallR;
	SurfaceParametrizationParams* surfaceParams = &(torus->m_surParams);
	SurfaceVerticesDescription* surfaceDescription = &(torus->m_surVerDesc);

	surfaceDescription->vertices.clear();
	surfaceDescription->indices.clear();
	// Get the points from torus parametrization
	float maxRotMainRadius = 2.0f * M_PI;
	float maxRotSmallRadius = 2.0f * M_PI;

	float densityX = (float)surfaceParams->densityX;
	float densityY = (float)surfaceParams->densityY;

	float bigRotStep = maxRotMainRadius / densityX;
	float smallRotStep = maxRotSmallRadius / densityY;

	std::vector<VertexPositionColor> vertices;
	
	float colorFrac = 0.85f;

	for (int x = 0; x < densityX; x++)
	{
		for (int y = 0; y < densityY; y++)
		{
			//Add vertices 
			//colorFrac = (float)y / (float)densityY;
			float curX = bigRotStep * x;
			float curY = smallRotStep * y;
			VertexPositionColor v11 = CalculateTorusVertex(R, r, curX, curY, colorFrac);
			surfaceDescription->vertices.push_back(v11);

			//Add indices representing edges
			surfaceDescription->indices.push_back(CalculateIndexForVertex(x, y, densityY));
			surfaceDescription->indices.push_back(CalculateIndexForVertex((x + 1) % (int)densityX, y, densityY));

			surfaceDescription->indices.push_back(CalculateIndexForVertex(x, y, densityY));
			surfaceDescription->indices.push_back(CalculateIndexForVertex(x, (y + 1) % (int)densityY, densityY));
		}
	}
}

unsigned int CalculateIndexForVertex(int x, int y, int innerLoopMaxVal)
{
	return x * innerLoopMaxVal + y;
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