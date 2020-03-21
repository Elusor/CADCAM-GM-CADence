#pragma once
#define _USE_MATH_DEFINES

#include <vector>
#include <math.h>
#include "geometryRep.h"
#include "Torus.h"

VertexPositionColor CalculateTorusVertex(float bigR, float smallR, float bigRRot, float smallRRot,float colorFrac);
void GetTorusVerticesLineList(Torus* torus);