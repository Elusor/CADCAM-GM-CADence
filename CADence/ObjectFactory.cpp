#include "ObjectFactory.h"
#include "Scene.h"
#include "mathUtils.h"
std::shared_ptr<Node> ObjectFactory::CreateBezierSurface(Scene* scene, int width, int height, XMFLOAT3 position, bool cylinder, float radius)
{

	BezierPatch*** patches;
	patches = new BezierPatch* * [width];

	
	for (int i = 0; i < width; i++) {
		patches[i] = new BezierPatch*[height];
	}

	auto p00 = CreateBezierPatch(scene);
	patches[0][0] = (BezierPatch*)p00->m_object.get();
	scene->AttachObject(p00);
	for (int i = 1; i < width; i++) {
		
		auto rightPoints = patches[i - 1][0]->GetPoints(BoundaryDirection::Right);
		auto point = CreateBezierPatch(scene,
			std::vector<std::weak_ptr<Node>>(),
			std::vector<std::weak_ptr<Node>>(),
			rightPoints);
		scene->AttachObject(point);
		patches[i][0] = (BezierPatch*)point->m_object.get();
	}

	for (int h = 1; h < height; h++)
	{
		auto botPoints = patches[0][h - 1]->GetPoints(BoundaryDirection::Top);
		auto p = CreateBezierPatch(scene,
			std::vector<std::weak_ptr<Node>>(),
			botPoints);
		scene->AttachObject(p);
		patches[0][h] = (BezierPatch*)p->m_object.get();

		for (int w = 1; w < width; w++)
		{
			auto bottomPoints = patches[w][h - 1]->GetPoints(BoundaryDirection::Top);
			auto leftPoints = patches[w-1][h]->GetPoints(BoundaryDirection::Right);
			auto innerPt = CreateBezierPatch(scene,
				std::vector<std::weak_ptr<Node>>(),
				bottomPoints,
				leftPoints);
			scene->AttachObject(innerPt);

			patches[w][h] = (BezierPatch*)innerPt->m_object.get();
		}
	}

	//auto p1 = CreateBezierPatch(scene);
	//BezierPatch* p = (BezierPatch*)p1->m_object.get();
	//scene->AttachObject(p1);

	//auto topp = p->GetPoints(BoundaryDirection::Bottom);
	//auto topf = p->GetPoints(BoundaryDirection::Right);
	//auto p2 = CreateBezierPatch(scene, topp);
	//BezierPatch* pa2 = (BezierPatch*)p2->m_object.get();
	//scene->AttachObject(p2);

	//auto p3 = CreateBezierPatch(scene, std::vector<std::weak_ptr<Node>>(), std::vector<std::weak_ptr<Node>>(), topf);
	//BezierPatch* pa3 = (BezierPatch*)p3->m_object.get();
	//scene->AttachObject(p3);

	//auto p3bot = pa3->GetPoints(BoundaryDirection::Bottom);
	//auto p2right = pa2->GetPoints(BoundaryDirection::Right);
	//auto p4 = CreateBezierPatch(scene, p3bot, std::vector<std::weak_ptr<Node>>(), p2right);
	//scene->AttachObject(p4);


	return std::shared_ptr<Node>();
}

std::shared_ptr<Node> ObjectFactory::CreateBezierPatch(
	Scene* scene,
	std::vector<std::weak_ptr<Node>> top,
	std::vector<std::weak_ptr<Node>> bottom,
	std::vector<std::weak_ptr<Node>> left,
	std::vector<std::weak_ptr<Node>> right
	)
{
	// This function is ungodly and should be smitten by god for it's sole existance (TODO : refactor)
	XMFLOAT3 pos = XMFLOAT3(0.f, 0.f, 0.f);
	float h = 5.0f;
	float w = 5.0f;

	std::vector<std::weak_ptr<Node>> u0;
	std::vector<std::weak_ptr<Node>> u1;
	std::vector<std::weak_ptr<Node>> u2;
	std::vector<std::weak_ptr<Node>> u3;
	std::weak_ptr<Node> c;

	for (int i = 0; i < 4; i++) {
		u0.push_back(c);
		u1.push_back(c);
		u2.push_back(c);
		u3.push_back(c);
	}

	bool allocateTop, allocateBot, allocateLeft, allocateRight;

	allocateTop = top.size() != 4;
	allocateBot = bottom.size() != 4;
	allocateLeft = left.size() != 4;
	allocateRight = right.size() != 4;

	if (allocateTop == false)
	{
		u0 = top;
		pos = u0[0].lock()->m_object->GetPosition();
	}

	if (allocateBot == false)
	{
		u3 = bottom;
		pos = XMF3SUB(u3[0].lock()->m_object->GetPosition(), XMFLOAT3(0.f, 0.0f, h * 3.f));
	}

	if (allocateLeft == false)
	{
		u0[0] = left[0];
		u1[0] = left[1];
		u2[0] = left[2];
		u3[0] = left[3];
		pos = u0[0].lock()->m_object->GetPosition();
	}

	if (allocateRight == false)
	{		
		u0[3] = right[0];
		u1[3] = right[1];
		u2[3] = right[2];
		u3[3] = right[3];		
		pos = XMF3SUB(u0[3].lock()->m_object->GetPosition(), XMFLOAT3(w * (float)3.f, 0.0f, 0.0f));
	}	
	
	// iterate through all vertices - if any of them are expired pointers - create new point	
	// Refactor this to not attach points to the scene - this should even have a reference to it


	for (int i = 0; i < 4; i++)
	{
		if (u0[i].expired())
		{
			auto pt = CreatePoint();
			scene->AttachObject(pt);		
			XMFLOAT3 ptPos = XMF3SUM(pos, XMFLOAT3(w * (float)i, 0.0f, 0.0f));
			pt->m_object->SetPosition(ptPos);
			u0[i] = pt;
		}

		if (u1[i].expired())
		{
			auto pt = CreatePoint();
			scene->AttachObject(pt);
			XMFLOAT3 ptPos = XMF3SUM(pos, XMFLOAT3(w * (float)i, 0.0f, h));
			pt->m_object->SetPosition(ptPos);
			u1[i] = pt;		
		}

		if (u2[i].expired())
		{
			auto pt = CreatePoint();
			scene->AttachObject(pt);
			XMFLOAT3 ptPos = XMF3SUM(pos, XMFLOAT3(w * (float)i, 0.0f, h * 2.f));
			pt->m_object->SetPosition(ptPos);
			u2[i] = pt;
		}

		if (u3[i].expired())
		{
			auto pt = CreatePoint();
			scene->AttachObject(pt);
			XMFLOAT3 ptPos = XMF3SUM(pos, XMFLOAT3(w * (float)i, 0.0f, h * 3.f));
			pt->m_object->SetPosition(ptPos);
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

	std::shared_ptr<Node> node = std::shared_ptr<Node>(new Node());
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
