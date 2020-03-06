#pragma once
#define _USE_MATH_DEFINES

#include <vector>
#include <math.h>
#include "mathStructures.h"
#include "cadStructures.h"

VertexPositionColor CalculateTorusVertex(float bigR, float smallR, float bigRRot, float smallRRot,float colorFrac);
SurfaceVerticesDescription* GetTorusVerticesLineList(float R, float r, SurfaceParametrizationParams surfaceParams);