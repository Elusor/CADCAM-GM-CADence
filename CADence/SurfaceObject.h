#pragma once
#include "MeshObject.h"

struct SurfaceObject : MeshObject
{
	SurfaceParametrizationParams m_surParams;
	bool CreateParamsGui() override;
};