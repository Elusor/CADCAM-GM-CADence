#pragma once
#include "MeshObject.h"

struct Parametrization2DArguments
{
	int densityX;
	int minDensityX;
	int maxDensityX;

	int densityY;
	int minDensityY;
	int maxDensityY;
};

class ParametrizedObject2D : public MeshObject<VertexPositionColor>
{
public:
	Parametrization2DArguments m_surParams;
	bool CreateParamsGui() override;
};