#include "ObjectFactory.h"

std::unique_ptr<Torus> ObjectFactory::CreateTorus(
	std::string name = "Torus",
	Transform transform,
	float bigR, float smallR, 
	float densityX, float densityY,
	float minDensityX, float minDensityY, 
	float maxDensityX, float maxDensityY)
{	
	Torus* t = new Torus();

	t->m_name = name;

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

	return std::unique_ptr<Torus>(t);
}
