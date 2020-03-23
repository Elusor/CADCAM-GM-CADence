#pragma once
#define _USE_MATH_DEFINES

#include <vector>
#include <math.h>
#include "objects.h"
#include "vertexStructures.h"

VertexPositionColor CalculateTorusVertex(float bigR, float smallR, float bigRRot, float smallRRot,float colorFrac);
void GetTorusVerticesLineList(Torus* torus);