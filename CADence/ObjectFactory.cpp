#include "ObjectFactory.h"
#include "torusGenerator.h"
std::unique_ptr<Torus> ObjectFactory::CreateTorus(
	Transform transform,
	std::string name,
	float bigR, float smallR, 
	float densityX, float densityY,
	float minDensityX, float minDensityY, 
	float maxDensityX, float maxDensityY)
{	
	Torus* t = new Torus();
#pragma region CreateObjectName
	t->m_name = name;

	if (m_torusCounter > 0)
	{
		t->m_name = "Torus " + std::to_string(m_torusCounter);

	}

	t->m_defaultName = t->m_name;
#pragma endregion	

	// Torus obly
	t->m_bigR = bigR;
	t->m_smallR = smallR;

	// Common for parametrized
	t->m_surParams.densityX = densityX;
	t->m_surParams.minDensityX = minDensityX;
	t->m_surParams.maxDensityX = maxDensityX;

	t->m_surParams.densityY = densityY;
	t->m_surParams.minDensityY = minDensityY;
	t->m_surParams.maxDensityY = maxDensityY;

	// Common for objects
	t->m_transform.m_pos = transform.m_pos;
	t->m_transform.m_rotation = transform.m_rotation;
	t->m_transform.m_scale = transform.m_scale;	

	GetTorusVerticesLineList(t);
	m_torusCounter++;

	return std::unique_ptr<Torus>(t);
}

std::unique_ptr<SpawnMarker> ObjectFactory::CreateSpawnMarker()
{	
	return std::unique_ptr<SpawnMarker>(new SpawnMarker());
}

std::unique_ptr<Point> ObjectFactory::CreatePoint(Transform transform)
{
	Point* p = new Point();

	std::string name = "Point ";
	if (m_pointCounter > 0)
	{
		name =  name + std::to_string(m_pointCounter);
	}

	p->m_name = name;
	p->m_defaultName = p->m_name;

	p->m_transform.m_pos = transform.m_pos;
	p->m_transform.m_rotation = transform.m_rotation;
	p->m_transform.m_scale = transform.m_scale;

	m_pointCounter++;
	return std::unique_ptr<Point>(p);
}
