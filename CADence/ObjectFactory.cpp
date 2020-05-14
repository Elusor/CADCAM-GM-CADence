#include "ObjectFactory.h"
#include "Scene.h"
std::shared_ptr<Node> ObjectFactory::CreateBezierPatch(
	std::vector<std::weak_ptr<Node>> top,
	std::vector<std::weak_ptr<Node>> bottom,
	std::vector<std::weak_ptr<Node>> left,
	std::vector<std::weak_ptr<Node>> right,
	Scene* scene)
{
	// This function is ungodly and should be smitten by god for it's sole existance (TODO : refactor)

	std::vector<std::weak_ptr<Node>> u0;
	std::vector<std::weak_ptr<Node>> u1;
	std::vector<std::weak_ptr<Node>> u2;
	std::vector<std::weak_ptr<Node>> u3;

	u0.reserve(4);
	u1.reserve(4);
	u2.reserve(4);
	u3.reserve(4);

	bool allocateTop, allocateBot, allocateLeft, allocateRight;

	allocateTop = top.size() != 4;
	allocateBot = bottom.size() != 4;
	allocateLeft = left.size() != 4;
	allocateRight = right.size() != 4;

	if (allocateTop == false)
	{
		u0 = top;
	}

	if (allocateBot == false)
	{
		u3 = bottom;
	}

	if (allocateLeft == false)
	{
		u0[0] = left[0];
		u1[0] = left[1];
		u2[0] = left[2];
		u3[0] = left[3];
	}

	if (allocateRight == false)
	{		
		u0[3] = right[0];
		u1[3] = right[1];
		u2[3] = right[2];
		u3[3] = right[3];		
	}	
	
	// iterate through all vertices - if any of them are expired pointers - create new point	
	// Refactor this to not attach points to the scene - this should even have a reference to it
	for (int i = 0; i < 4; i++)
	{
		if (u0[i].expired)
		{
			auto pt = CreatePoint();
			scene->AttachObject(pt);
			u0[i] = pt;
		}

		if (u1[i].expired)
		{
			auto pt = CreatePoint();
			scene->AttachObject(pt);
			u1[i] = pt;		
		}

		if (u2[i].expired)
		{
			auto pt = CreatePoint();
			scene->AttachObject(pt);
			u2[i] = pt;
		}

		if (u3[i].expired)
		{
			auto pt = CreatePoint();
			scene->AttachObject(pt);
			u3[i] = pt;
		}		
	}

	BezierPatch* patch = new BezierPatch(u0, u1, u2, u3);
	std::string name = "Bezier Patch";
	if (m_bezierPatchCounter > 0)
	{
		name = name + " " + std::to_string(m_bezierPatchCounter);
	}

	patch->m_name = patch->m_defaultName = name;

	std::shared_ptr<Node> node = std::make_shared<Node>(patch);
	auto object = std::unique_ptr<Object>(patch);
	node->m_object = move(object);
	m_bezierPatchCounter++;
	return node;
}

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
	n->m_object->SetPosition(DirectX::XMFLOAT3(0.0f, 0.05f, 0.0f));
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
