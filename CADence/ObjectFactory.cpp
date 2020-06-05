#include "ObjectFactory.h"
#include "Scene.h"
#include "mathUtils.h"
std::shared_ptr<Node> ObjectFactory::CreateBezierSurface(Scene* scene,
	int width, int height, XMFLOAT3 position,
	bool cylinder, float sizeW, float sizeH,
	SurfaceWrapDirection wrapDir)
{
	int widthPointCount = 3 * width + 1;
	int heightPointCount = 3 * height + 1;

	int wrappedHeight = heightPointCount;
	int wrappedWidth = widthPointCount;
	
	if (cylinder)
	{
		if (wrapDir == SurfaceWrapDirection::Height)
		{
			wrappedHeight--;
		}

		if (wrapDir == SurfaceWrapDirection::Width)
		{
			wrappedWidth--;
		}
	}

	std::shared_ptr<Node>** points = new std::shared_ptr<Node>*[widthPointCount];
	for (int i = 0; i < widthPointCount; i++) {
		points[i] = new std::shared_ptr<Node>[heightPointCount];
	}

	for (int w = 0; w < wrappedWidth; w++) {
		for (int h = 0; h < wrappedHeight; h++) {
			auto pt = CreatePoint();
			scene->AttachObject(pt);
			pt->m_object->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
			points[w][h] = pt;
		}
	}

	if (cylinder)
	{
		if (wrapDir == SurfaceWrapDirection::Height)
		{
			for (int w = 0; w < wrappedWidth; w++)
			{
				points[w][heightPointCount - 1] = points[w][0];
			}
		}

		if (wrapDir == SurfaceWrapDirection::Width)
		{
			points[widthPointCount - 1] = points[0];
		}
	}
	
	std::vector<std::shared_ptr<Node>> surfPatches = std::vector<std::shared_ptr<Node>>();
	BezierPatch*** patches;
	patches = new BezierPatch** [width];

	for (int i = 0; i < width; i++) {
		patches[i] = new BezierPatch*[height];
	}

	float patchSizeW = sizeW / (float)width;
	float patchSizeH = sizeH / (float)height;

	for (int patchW = 0; patchW < width; patchW++) {
		for (int patchH = 0; patchH < height; patchH++) {

			std::vector<std::weak_ptr<Node>> top, bot, topMid, botMid;
			// determine top bot left and right
			int startIdxW = 3 * patchW;
			int startIdxH = 3 * patchH;
			top =	 { points[startIdxW][startIdxH + 3], points[startIdxW + 1][startIdxH + 3], points[startIdxW + 2][startIdxH + 3], points[startIdxW + 3][startIdxH + 3] };
			topMid = { points[startIdxW][startIdxH + 2], points[startIdxW + 1][startIdxH + 2], points[startIdxW + 2][startIdxH + 2], points[startIdxW + 3][startIdxH + 2] };
			botMid = { points[startIdxW][startIdxH + 1], points[startIdxW + 1][startIdxH + 1], points[startIdxW + 2][startIdxH + 1], points[startIdxW + 3][startIdxH + 1] };
			bot =	 { points[startIdxW][startIdxH], points[startIdxW + 1][startIdxH], points[startIdxW + 2][startIdxH], points[startIdxW + 3][startIdxH] };

			auto patch = CreateBezierPatch(scene, top, topMid, botMid, bot);
			patches[patchW][patchH] = (BezierPatch*)patch->m_object.get();
			surfPatches.push_back(patch);
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

	//TODO release all dynamically set up resources

	for (int i = 0; i < wrappedWidth; i++)
	{
		delete[] (points[i]);
	}
	delete[] points;

	for (int i = 0; i < width; i++)
	{
		delete[] (patches[i]);
	}
	delete[] patches;


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
	std::vector<std::weak_ptr<Node>> topMid,
	std::vector<std::weak_ptr<Node>> botMid,
	std::vector<std::weak_ptr<Node>> bot
	)
{
	XMFLOAT3 pos = XMFLOAT3(0.f, 0.f, 0.f);

	std::vector<std::weak_ptr<Node>> u0;
	std::vector<std::weak_ptr<Node>> u1;
	std::vector<std::weak_ptr<Node>> u2;
	std::vector<std::weak_ptr<Node>> u3;
	
	bool allocateTop, allocateBot, allocateLeft, allocateRight;

	allocateTop = top.size() != 4;
	allocateBot = bot.size() != 4;
	allocateLeft = topMid.size() != 4;
	allocateRight = botMid.size() != 4;

	for (int i = 0; i < 4; i++)
	{
		u0.push_back(top[i]);
		u1.push_back(topMid[i]);
		u2.push_back(botMid[i]);
		u3.push_back(bot[i]);
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
