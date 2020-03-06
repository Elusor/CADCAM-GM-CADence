#pragma once
#define _USE_MATH_DEFINES

#include <vector>
#include <math.h>
#include "mathStructures.h"

VertexPositionColor CalculateTorusVertex(float bigR, float smallR, float bigRRot, float smallRRot,float colorFrac);
std::vector<VertexPositionColor> GetTorusVerticesLineList(float R, float r, int densityX, int densityY);

//probably move density X and density Y to some SurfaceParameters class/struct
std::vector<VertexPositionColor> GetTorusVerticesLineList(float R, float r, int densityX, int densityY)
{
	// Get the points from torus parametrization
	float maxRotMainRadius = 2.0f * M_PI;
	float maxRotSmallRadius = 2.0f * M_PI;

	float bigRotStep = maxRotMainRadius / (float)densityX;
	float smallRotStep = maxRotSmallRadius / (float)densityY;


	std::vector<VertexPositionColor> vertices;
	// Calculate mainRotStep x smallRotStep rectangles
	for (int x = 1; x <= densityX; x++)
	{
		for (int y = 1; y <= densityY; y++)
		{
			
			float x1 = bigRotStep * x;
			float y1 = smallRotStep * y;
			float x0 = x1 - bigRotStep;
			float y0 = y1 - smallRotStep;

			float colorFrac = 0.85f;

			//calculate for x, y
			VertexPositionColor v11 = CalculateTorusVertex(R, r, x1, y1, colorFrac);
			//calculate for x-1, y										 colorFraq
			VertexPositionColor v01 = CalculateTorusVertex(R, r, x0, y1, colorFrac);
			//calculate for x, y-1										 colorFraq
			VertexPositionColor v10 = CalculateTorusVertex(R, r, x1, y0, colorFrac);
			//calculate for x-1, y-1									 colorFraq
			VertexPositionColor v00 = CalculateTorusVertex(R, r, x0, y0, colorFrac);

			// Add points in Line List Primitive Topology
			vertices.push_back(v00);
			vertices.push_back(v01);

			vertices.push_back(v01);
			vertices.push_back(v11);

			vertices.push_back(v11);
			vertices.push_back(v10);

			vertices.push_back(v10);
			vertices.push_back(v00);
		}
	}

	return vertices;
}

VertexPositionColor CalculateTorusVertex(float bigR, float smallR, float bigRRot, float smallRRot, float colorFrac)
{
	
	float x = (smallR * cosf(smallRRot) + bigR) * cosf(bigRRot);
	float z = (smallR * cosf(smallRRot) + bigR) * sinf(bigRRot);
	float y = smallR * sinf(smallRRot);
	VertexPositionColor v {
		{x,y,z},
		{colorFrac,colorFrac,colorFrac}
	};
	return v;
}