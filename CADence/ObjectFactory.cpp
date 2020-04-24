#include "ObjectFactory.h"

std::shared_ptr<Node> ObjectFactory::CreateInterpolBezierCurveC2(std::vector<std::weak_ptr<Node>> controlPoints)
{
	auto points = FilterObjectTypes(typeid(Point), controlPoints);
	InterpolationBezierCurveC2* bC = new InterpolationBezierCurveC2(points);
	std::string name = "Bezier Curve";

	if (m_bezierCurveCounter > 0)
	{
		name = name + " " + std::to_string(m_bezierCurveCounter);
	}
	bC->m_name = bC->m_defaultName = name;

	std::shared_ptr<Node> node = std::shared_ptr<Node>(new GroupNode(points));
	node->m_object = std::unique_ptr<Object>(bC);
	bC->m_parent = node;
	m_bezierCurveCounter++;
	return node;
}

std::shared_ptr<Node> ObjectFactory::CreateBezierCurveC2(std::vector<std::weak_ptr<Node>> controlPoints, BezierBasis basis)
{

	auto points = FilterObjectTypes(typeid(Point), controlPoints);
	BezierCurveC2* bC = new BezierCurveC2(points, basis);
	std::string name = "Bezier Curve";
	
	if (m_bezierCurveCounter > 0)
	{
		name = name + " " + std::to_string(m_bezierCurveCounter);
	}
	bC->m_name = bC->m_defaultName = name;

	std::shared_ptr<Node> node = std::shared_ptr<Node>(new GroupNode(points));
	node->m_object = std::unique_ptr<Object>(bC);
	bC->m_parent = node;
	bC->RecalculateBasisPoints();
	m_bezierCurveCounter++;
	return node;
}

std::shared_ptr<Node> ObjectFactory::CreateBezierCurve(std::vector<std::weak_ptr<Node>> controlPoints)
{
	// TODO [MG] check if all nodes are Points
	
	auto points = FilterObjectTypes(typeid(Point), controlPoints);

	BezierCurve* bC = new BezierCurve(points);

	std::string name = "Bezier Curve";
	if (m_bezierCurveCounter > 0)
	{
		name = name + " " + std::to_string(m_bezierCurveCounter);
	}

	bC->m_name = bC->m_defaultName = name;

	std::shared_ptr<Node> node = std::shared_ptr<Node>(new GroupNode(points));
	node->m_object = std::unique_ptr<Object>(bC);;	
	bC->m_parent = node;
	m_bezierCurveCounter++;
	return node;
}


std::shared_ptr<Node> ObjectFactory::CreateSceneGrid(int gridSize)
{
	std::shared_ptr<Node> n = std::shared_ptr<Node>(new Node());
	n->m_object = std::unique_ptr<SceneGrid>(new SceneGrid(gridSize));
	n->m_object->m_parent = n;
	return n;
}

std::shared_ptr<Node> ObjectFactory::CreateTorus(
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

	// Move to a section general for all objects
	t->SetTransform(transform);

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

	std::shared_ptr<Node> n = std::shared_ptr<Node>(new Node());
	n->m_object = std::unique_ptr<Torus>(t);
	t->m_parent = n;
	return n;
}

std::shared_ptr<Node> ObjectFactory::CreateSpawnMarker()
{	
	std::shared_ptr<Node> n = std::shared_ptr<Node>(new Node());
	n->m_object = std::unique_ptr<SpawnMarker>(new SpawnMarker());
	n->m_object->m_parent = n;
	return n;
}

std::shared_ptr<Node> ObjectFactory::CreatePoint(Transform transform)
{
	Point* p = new Point();

	std::string name = "Point ";
	if (m_pointCounter > 0)
	{
		name =  name + std::to_string(m_pointCounter);
	}

	p->m_name = name;
	p->m_defaultName = p->m_name;
	p->SetTransform(transform);

	m_pointCounter++;

	std::shared_ptr<Node> n = std::shared_ptr<Node>(new Node());
	n->m_object = std::unique_ptr<Point>(p);
	p->m_parent = n;
	return n;
}

std::vector<std::weak_ptr<Node>> ObjectFactory::FilterObjectTypes(const type_info& typeId, std::vector<std::weak_ptr<Node>> nodes)
{
	auto it = nodes.begin();

	while (it != nodes.end())
	{
		if (auto point = it->lock())
		{
			if (typeid(*(point->m_object)) != typeId) //Object is not point
			{
				it = nodes.erase(it);
			}
			else {
				it++;
			}
		}
	}

	return nodes;
}
