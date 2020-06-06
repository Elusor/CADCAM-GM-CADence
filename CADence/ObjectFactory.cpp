#include "ObjectFactory.h"
#include "Scene.h"
#include "mathUtils.h"
std::shared_ptr<Node> ObjectFactory::CreateBezierSurface(Scene* scene,
	int patchesW, int patchesH, XMFLOAT3 middlePosition,
	bool cylinder, float width, float height,
	SurfaceWrapDirection wrapDir)
{
	int widthPointCount = 3 * patchesW + 1;
	int heightPointCount = 3 * patchesH + 1;

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

	std::shared_ptr<Node>** points = new std::shared_ptr<Node> * [widthPointCount];
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
	patches = new BezierPatch * *[patchesW];

	for (int i = 0; i < patchesW; i++) {
		patches[i] = new BezierPatch * [patchesH];
	}

	float patchSizeW = width / (float)patchesW;
	float patchSizeH = height / (float)patchesH;

	for (int patchW = 0; patchW < patchesW; patchW++) {
		for (int patchH = 0; patchH < patchesH; patchH++) {

			std::vector<std::weak_ptr<Node>> top, bot, topMid, botMid;
			// determine top bot left and right
			int startIdxW = 3 * patchW;
			int startIdxH = 3 * patchH;
			top = { points[startIdxW][startIdxH + 3], points[startIdxW + 1][startIdxH + 3], points[startIdxW + 2][startIdxH + 3], points[startIdxW + 3][startIdxH + 3] };
			topMid = { points[startIdxW][startIdxH + 2], points[startIdxW + 1][startIdxH + 2], points[startIdxW + 2][startIdxH + 2], points[startIdxW + 3][startIdxH + 2] };
			botMid = { points[startIdxW][startIdxH + 1], points[startIdxW + 1][startIdxH + 1], points[startIdxW + 2][startIdxH + 1], points[startIdxW + 3][startIdxH + 1] };
			bot = { points[startIdxW][startIdxH], points[startIdxW + 1][startIdxH], points[startIdxW + 2][startIdxH], points[startIdxW + 3][startIdxH] };

			auto patch = CreateBezierPatch(scene, top, topMid, botMid, bot);
			patches[patchW][patchH] = (BezierPatch*)patch->m_object.get();
			surfPatches.push_back(patch);
		}
	}

	// determine pointStepW and pointStepH
	// Move to grid
		
	for (int w = 0; w < wrappedWidth; w++)
	{
		for (int h = 0; h < wrappedHeight; h++)
		{
			if (cylinder)
			{
				if (wrapDir == SurfaceWrapDirection::Width)
				{
					// case width wrap
					float currentArg = (float)w / (float)wrappedWidth * XM_2PI;
					float radius = width;
					float pointStepLen = height / ((float)patchesH * 3.0f);
					points[w][h]->m_object->SetPosition(
						XMFLOAT3(
							sinf(currentArg) * radius,
							cosf(currentArg) * radius,
							(float)h * pointStepLen - height / 2.f)
					);
				}
				else {
					float currentArg = (float)h / (float)wrappedHeight * XM_2PI;
					float radius = height;
					float pointStepLen = width / ((float)patchesW * 3.0f);
					points[w][h]->m_object->SetPosition(
						XMFLOAT3(
						(float)w * pointStepLen - width / 2.f,
							cosf(currentArg) * radius,
							sinf(currentArg) * radius)
					);
				}
			}
			else 
			{
				float pointStepW = width / ((float)patchesW * 3.0f);
				float pointStepH = height / ((float)patchesH * 3.0f);

				
				points[w][h]->m_object->SetPosition(
					XMFLOAT3(
						(float)w * pointStepW - width / 2.f,
						0.0f,
						(float)h * pointStepH - height / 2.f));
				
			}
			// move all point so they center of the surface is in the cursor
			points[w][h]->m_object->SetPosition(
				XMF3SUM(points[w][h]->m_object->GetPosition(), middlePosition));
			
		}
	}			
	
	for (int patchW = 0; patchW < patchesW; patchW++) {
		for (int patchH = 0; patchH < patchesH; patchH++) {
			patches[patchW][patchH]->UpdateObject();
		}
	}	

	// Relese auxiliary data
	for (int i = 0; i < wrappedWidth; i++)
	{
		delete[] (points[i]);
	}
	delete[] points;

	for (int i = 0; i < patchesW; i++)
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

std::shared_ptr<Node> ObjectFactory::CreateBezierSurfaceC2(Scene* scene,
	int patchesW, int patchesH, XMFLOAT3 middlePosition,
	bool cylinder, float width, float height,
	SurfaceWrapDirection wrapDir)
{
	int widthPointCount = 3 + patchesW;
	int heightPointCount = 3 + patchesH;

	int wrappedHeight = heightPointCount;
	int wrappedWidth = widthPointCount;

	if (cylinder)
	{
		if (wrapDir == SurfaceWrapDirection::Height)
		{
			wrappedHeight -= 3;
		}

		if (wrapDir == SurfaceWrapDirection::Width)
		{
			wrappedWidth -= 3;
		}
	}

	std::shared_ptr<Node>** points = new std::shared_ptr<Node> * [widthPointCount];
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

	// Wrap
	if (cylinder)
	{
		// Wrap 3 last points
		if (wrapDir == SurfaceWrapDirection::Height)
		{
			for (int w = 0; w < wrappedWidth; w++)
			{
				points[w][heightPointCount - 1] = points[w][2 % wrappedHeight];
				points[w][heightPointCount - 2] = points[w][1 % wrappedHeight];
				points[w][heightPointCount - 3] = points[w][0 % wrappedHeight];
			}
		}

		if (wrapDir == SurfaceWrapDirection::Width)
		{
			points[widthPointCount - 1] = points[2 % wrappedWidth];
			points[widthPointCount - 2] = points[1 % wrappedWidth];
			points[widthPointCount - 3] = points[0 % wrappedWidth];
		}
	}

	std::vector<std::shared_ptr<Node>> surfPatches = std::vector<std::shared_ptr<Node>>();
	BezierPatch*** patches;
	patches = new BezierPatch * *[patchesW];

	for (int i = 0; i < patchesW; i++) {
		patches[i] = new BezierPatch * [patchesH];
	}

	float patchSizeW = width / (float)patchesW;
	float patchSizeH = height / (float)patchesH;

	for (int patchW = 0; patchW < patchesW; patchW++) {
		for (int patchH = 0; patchH < patchesH; patchH++) {

			std::vector<std::weak_ptr<Node>> top, bot, topMid, botMid;
			// determine top bot left and right
			int startIdxW = patchW;
			int startIdxH = patchH;
			top = { points[startIdxW][startIdxH + 3], points[startIdxW + 1][startIdxH + 3], points[startIdxW + 2][startIdxH + 3], points[startIdxW + 3][startIdxH + 3] };
			topMid = { points[startIdxW][startIdxH + 2], points[startIdxW + 1][startIdxH + 2], points[startIdxW + 2][startIdxH + 2], points[startIdxW + 3][startIdxH + 2] };
			botMid = { points[startIdxW][startIdxH + 1], points[startIdxW + 1][startIdxH + 1], points[startIdxW + 2][startIdxH + 1], points[startIdxW + 3][startIdxH + 1] };
			bot = { points[startIdxW][startIdxH], points[startIdxW + 1][startIdxH], points[startIdxW + 2][startIdxH], points[startIdxW + 3][startIdxH] };

			auto patch = CreateBezierPatchC2(scene, top, topMid, botMid, bot);
			patches[patchW][patchH] = (BezierPatch*)patch->m_object.get();
			surfPatches.push_back(patch);
		}
	}

	// determine pointStepW and pointStepH
	// Move to grid

	for (int w = 0; w < wrappedWidth; w++)
	{
		for (int h = 0; h < wrappedHeight; h++)
		{
			if (cylinder)
			{
				if (wrapDir == SurfaceWrapDirection::Width)
				{
					// case width wrap
					float currentArg = (float)w / (float)wrappedWidth * XM_2PI;
					float radius = width;
					float pointStepLen = height / ((float)heightPointCount-1);
					points[w][h]->m_object->SetPosition(
						XMFLOAT3(
							sinf(currentArg) * radius,
							cosf(currentArg) * radius,
							(float)h * pointStepLen - height / 2.f)
					);
				}
				else {
					float currentArg = (float)h / (float)wrappedHeight * XM_2PI;
					float radius = height;
					float pointStepLen = width / ((float)widthPointCount-1);
					points[w][h]->m_object->SetPosition(
						XMFLOAT3(
						(float)w * pointStepLen - width / 2.f,
							cosf(currentArg) * radius,
							sinf(currentArg) * radius)
					);
				}
			}
			else
			{
				float pointStepW = width / ((float)widthPointCount-1);
				float pointStepH = height / ((float)heightPointCount-1);


				points[w][h]->m_object->SetPosition(
					XMFLOAT3(
					(float)w * pointStepW - width / 2.f,
						0.0f,
						(float)h * pointStepH - height / 2.f));

			}
			// move all point so they center of the surface is in the cursor
			points[w][h]->m_object->SetPosition(
				XMF3SUM(points[w][h]->m_object->GetPosition(), middlePosition));

		}
	}

	for (int patchW = 0; patchW < patchesW; patchW++) {
		for (int patchH = 0; patchH < patchesH; patchH++) {
			patches[patchW][patchH]->UpdateObject();
		}
	}

	// Relese auxiliary data
	for (int i = 0; i < wrappedWidth; i++)
	{
		delete[](points[i]);
	}
	delete[] points;

	for (int i = 0; i < patchesW; i++)
	{
		delete[](patches[i]);
	}
	delete[] patches;


	BezierSurfaceC0* surface = new BezierSurfaceC0(surfPatches);

	std::string name = "Bezier Surface C2";
	if (m_bezierSurfaceCounter > 0)
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

std::shared_ptr<Node> ObjectFactory::CreateBezierPatchC2(
	Scene* scene,
	std::vector<std::weak_ptr<Node>> top,
	std::vector<std::weak_ptr<Node>> topMid,
	std::vector<std::weak_ptr<Node>> botMid,
	std::vector<std::weak_ptr<Node>> bot)
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

	BezierPatchC2* patch = new BezierPatchC2(u0, u1, u2, u3);
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
