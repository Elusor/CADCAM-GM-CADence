#pragma once
#include <string>
#include "cadStructures.h"
#include "torusGenerator.h"
class ObjectFactory
{

public:
	std::unique_ptr<Torus> CreateTorus(
		Transform transform = Transform(),
		std::string name = "Torus",		
		float bigR = 8, float smallR = 3,
		float densityX = 10, float densityY = 10,
		float minDensityX = 3, float minDensityY = 3,
		float maxDensityX = 30, float maxDensityY = 30);

	std::unique_ptr<SpawnMarker> CreateSpawnMarker();

private:
	int m_torusCounter = 0;

};