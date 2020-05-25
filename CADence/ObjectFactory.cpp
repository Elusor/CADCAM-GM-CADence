#include "ObjectFactory.h"
#include "Scene.h"
#include "mathUtils.h"
std::shared_ptr<Node> ObjectFactory::CreateBezierSurface(Scene* scene, int width, int height, XMFLOAT3 position, bool cylinder, float sizeW, float sizeH)
{
	std::vector<std::shared_ptr<Node>> surfPatches = std::vector<std::shared_ptr<Node>>();
	BezierPatch*** patches;
	patches = new BezierPatch* * [width];

	float patchSizeW = sizeW / (float)width;
	float patchSizeH = sizeH / (float)height;

	for (int i = 0; i < width; i++) {
		patches[i] = new BezierPatch*[height];
	}

	// Create firts patch
	auto p00 = CreateBezierPatch(scene);
	patches[0][0] = (BezierPatch*)p00->m_object.get();
	surfPatches.push_back(p00);
	//scene->AttachObject(p00);


	//Create First Row
	for (int i = 1; i < width-1; i++) {
		
		auto leftPoints = patches[i - 1][0]->GetPoints(BoundaryDirection::Right);
		auto point = CreateBezierPatch(scene,
			std::vector<std::weak_ptr<Node>>(),
			std::vector<std::weak_ptr<Node>>(),
			leftPoints);
		surfPatches.push_back(point);
		//scene->AttachObject(point);
		patches[i][0] = (BezierPatch*)point->m_object.get();
	}

	// Create the rest but one
	for (int h = 1; h < height; h++)
	{
		auto botPoints = patches[0][h - 1]->GetPoints(BoundaryDirection::Top);
		auto p = CreateBezierPatch(scene,
			std::vector<std::weak_ptr<Node>>(),
			botPoints);
		surfPatches.push_back(p);
		//scene->AttachObject(p);
		patches[0][h] = (BezierPatch*)p->m_object.get();

		for (int w = 1; w < width-1; w++)
		{
			auto bottomPoints = patches[w][h - 1]->GetPoints(BoundaryDirection::Top);
			auto leftPoints = patches[w-1][h]->GetPoints(BoundaryDirection::Right);
			auto innerPt = CreateBezierPatch(scene,
				std::vector<std::weak_ptr<Node>>(),
				bottomPoints,
				leftPoints);
			surfPatches.push_back(innerPt);
			//scene->AttachObject(innerPt);

			patches[w][h] = (BezierPatch*)innerPt->m_object.get();
		}
	}

	if (width > 1)
	{	
	// Create the last one 
	// Attach the first 
	auto leftPoints = patches[width-2][0]->GetPoints(BoundaryDirection::Right);
	
	std::vector<std::weak_ptr<Node>> rightPoints;
	if(cylinder)
		rightPoints = patches[0][0]->GetPoints(BoundaryDirection::Left);
	else {
		rightPoints = std::vector<std::weak_ptr<Node>>();
	}

	auto firstConnector = CreateBezierPatch(scene, std::vector<std::weak_ptr<Node>>(), std::vector<std::weak_ptr<Node>>(), leftPoints, rightPoints);
	surfPatches.push_back(firstConnector);
	//scene->AttachObject(p);
	patches[width-1][0] = (BezierPatch*)firstConnector->m_object.get();

	for (int h = 1; h < height; h++)
	{
		auto botPoints = patches[width-1][h-1]->GetPoints(BoundaryDirection::Top);
		
		std::vector<std::weak_ptr<Node>> rightPoints;
		if (cylinder)
			rightPoints = patches[0][h]->GetPoints(BoundaryDirection::Left);
		else {
			rightPoints = std::vector<std::weak_ptr<Node>>();
		}

		auto leftPoints = patches[width-2][h]->GetPoints(BoundaryDirection::Right);
		auto connector = CreateBezierPatch(scene, std::vector<std::weak_ptr<Node>>(), botPoints, leftPoints, rightPoints);
		surfPatches.push_back(connector);
		//scene->AttachObject(p);
		patches[width-1][h] = (BezierPatch*)connector->m_object.get();

	}

	}
	for (int w = 0; w < width; w++)
	{
		for (int h = 0; h < height; h++)
		{

			BezierPatch* patch = patches[w][h];
			if (cylinder == false)
			{				
				float baseX = position.x + (float)w * patchSizeW;
				float baseY = position.y + 0.0f;
				float baseZ = position.z + (float)h * patchSizeH;
				auto u0 = patch->GetPoints(RowPlace::First);


				for (int i = 0; i < 4; i++)
				{
					float x = baseX + (float)patchSizeW * (float)i / 3.f;
					float z = baseZ + (float)patchSizeH;
					u0[i].lock()->m_object->SetPosition(XMFLOAT3(x, baseY, z));
				}

				auto U1 = patch->GetPoints(RowPlace::Second);
				for (int i = 0; i < 4; i++)
				{
					float x = baseX + (float)patchSizeW * (float)i / 3.f;
					float z = baseZ + (float)patchSizeH * 2.f / 3.f;
					U1[i].lock()->m_object->SetPosition(XMFLOAT3(x, baseY, z));
				}

				auto u2 = patch->GetPoints(RowPlace::Third);
				for (int i = 0; i < 4; i++)
				{
					float x = baseX + (float)patchSizeW * (float)i / 3.f;
					float z = baseZ + (float)patchSizeH * 1.f / 3.f;
					u2[i].lock()->m_object->SetPosition(XMFLOAT3(x, baseY, z));

				}

				auto u3 = patch->GetPoints(RowPlace::Fourth);
				for (int i = 0; i < 4; i++)
				{
					float x = baseX + (float)patchSizeW * (float)i / 3.f;
					float z = baseZ;
					u3[i].lock()->m_object->SetPosition(XMFLOAT3(x, baseY, z));
				}
			}
			else {

				float baseX = position.x;
				float baseY = position.y;
				float baseZ = position.z + (float)h * patchSizeH;
				
				float step = XM_2PI / (float)width;
				auto u0 = patch->GetPoints(RowPlace::First);
				for (int i = 0; i < 4; i++)
				{
					float angle = (float)i / 3.f * step + w * step;
					float x = baseX + sinf(angle) * sizeW;
					float y = baseY + cosf(angle) * sizeW;
					float z = baseZ + (float)patchSizeH;
					u0[i].lock()->m_object->SetPosition(XMFLOAT3(x, y, z));
				}

				auto U1 = patch->GetPoints(RowPlace::Second);
				for (int i = 0; i < 4; i++)
				{
					float angle = (float)i / 3.f * step + w * step;
					float x = baseX + sinf(angle) * sizeW;
					float y = baseY + cosf(angle) * sizeW;
					float z = baseZ + (float)patchSizeH * 2.f / 3.f;
					U1[i].lock()->m_object->SetPosition(XMFLOAT3(x, y, z));
				}

				auto u2 = patch->GetPoints(RowPlace::Third);
				for (int i = 0; i < 4; i++)
				{
					float angle = (float)i / 3.f * step + w * step;
					float x = baseX + sinf(angle) * sizeW;
					float y = baseY + cosf(angle) * sizeW;
					float z = baseZ + (float)patchSizeH * 1.f / 3.f;
					u2[i].lock()->m_object->SetPosition(XMFLOAT3(x, y, z));

				}

				auto u3 = patch->GetPoints(RowPlace::Fourth);
				for (int i = 0; i < 4; i++)
				{
					float angle = (float)i / 3.f * step + w * step;
					float x = baseX + sinf(angle) * sizeW;
					float y = baseY + cosf(angle) * sizeW;
					float z = baseZ;
					u3[i].lock()->m_object->SetPosition(XMFLOAT3(x, y, z));
				}
			}

			patch->UpdateObject();
		}
	}


	BezierSurfaceC0* surface = new BezierSurfaceC0(surfPatches);

	std::string name = "Bezier Surface";
	if (m_bezierSurfaceCounter> 0)
	{
		name = name + " " + std::to_string(m_bezierSurfaceCounter);
	}
	surface->m_name = surface->m_defaultName = name;
	m_bezierSurfaceCounter++;

	std::shared_ptr<Node> node = std::shared_ptr<Node>(new Node());
	auto object = std::unique_ptr<Object>(surface);
	node->m_object = move(object);
	scene->AttachObject(node);
	return node;
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

void ObjectFactory::ClearScene()
{
	m_bezierSurfaceCounter = 0;
	m_bezierPatchCounter = 0;
	m_bezierCurveCounter = 0;
	m_torusCounter = 0;
	m_pointCounter = 0;
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
