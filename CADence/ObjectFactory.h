#pragma once
#include <string>
#include "cadStructures.h"
#include "torusGenerator.h"
class ObjectFactory
{

public:
	std::unique_ptr<Torus> CreateTorus(
		std::string name = "Torus",
		Transform transform = Transform(),
		float bigR = 8, float smallR = 3,
		float densityX = 10, float densityY = 10,
		float minDensityX = 3, float minDensityY = 3,
		float maxDensityX = 30, float maxDensityY = 30);

private:
	int m_torusCounter = 0;

};