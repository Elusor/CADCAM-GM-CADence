#pragma once
#include "MeshObject.h"

struct SurfaceParametrizationParams
{
	int densityX;
	int minDensityX;
	int maxDensityX;

	int densityY;
	int minDensityY;
	int maxDensityY;
};

struct SurfaceObject : MeshObject
{
	SurfaceParametrizationParams m_surParams;
	bool CreateParamsGui() override;
};