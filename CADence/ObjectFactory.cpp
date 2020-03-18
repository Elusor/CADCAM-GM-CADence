#include "ObjectFactory.h"

std::unique_ptr<Torus> ObjectFactory::CreateTorus(
	Transform transform,
	std::string name,
	float bigR, float smallR, 
	float densityX, float densityY,
	float minDensityX, float minDensityY, 
	float maxDensityX, float maxDensityY)
{	
	Torus* t = new Torus();

	t->m_name = name;

	if (m_torusCounter > 0)
	{
		t->m_name = "Torus " + std::to_string(m_torusCounter);
	}

	// Move to a section general for all objects
	t->m_transform.m_pos = transform.m_pos;
	t->m_transform.m_rotation = transform.m_rotation;
	t->m_transform.m_scale = transform.m_scale;

	t->m_bigR = bigR;
	t->m_smallR = smallR;

	t->m_surParams.densityX = densityX;
	t->m_surParams.minDensityX = minDensityX;
	t->m_surParams.maxDensityX = maxDensityX;

	t->m_surParams.densityY = densityY;
	t->m_surParams.minDensityY = minDensityY;
	t->m_surParams.maxDensityY = maxDensityY;

	GetTorusVerticesLineList(t);
	m_torusCounter++;

	return std::unique_ptr<Torus>(t);
}

std::unique_ptr<SpawnMarker> ObjectFactory::CreateSpawnMarker()
{	
	return std::unique_ptr<SpawnMarker>(new SpawnMarker());
}
