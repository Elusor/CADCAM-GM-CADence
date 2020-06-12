#include "sceneImporter.h"
#include "IOExceptions.h"
#include "ArgumentExceptions.h"
#include <cassert>
#include <codecvt>
#include <algorithm>

SceneImporter::SceneImporter(Scene* scene, GuiManager* guiManager)
{
	m_guiManager = guiManager;
	m_scene = scene;
	m_factory = scene->m_objectFactory.get();
}

bool SceneImporter::Import(std::wstring wpath)
{
	bool fileLoadedCorrectly = true;

	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;		
	auto path = converterX.to_bytes(wpath);
	tinyxml2::XMLDocument doc;
	auto res = doc.LoadFile((path).c_str());
	if (res != tinyxml2::XMLError::XML_SUCCESS)
		return false;

	m_scene->ClearScene();

	try {

		tinyxml2::XMLElement* scene = doc.FirstChildElement("Scene");

		if (scene == nullptr)
		{
			throw SceneClassifierNotFoundException();			
		}
		else {
			m_loadedPoints.clear();		
			m_loadedObjects.clear();
		}

		tinyxml2::XMLElement* element = scene->FirstChildElement();
	
		while (element != nullptr)
		{
			ProcessElement(element);
			element = element->NextSiblingElement();
		}
	}
	catch (std::exception & e)
	{
		m_guiManager->EnableCustomModal(e.what(), "Reading error");
		fileLoadedCorrectly = false;
	}
	
	if (fileLoadedCorrectly)
	{
		SwitchToLoadedScene();
		m_loadedObjects.clear();
		m_loadedPoints.clear();
	}
	
	return fileLoadedCorrectly;
}

void SceneImporter::ProcessElement(tinyxml2::XMLElement* element)
{
	assert(element != nullptr);

	auto name = std::string(element->Name());
	
	if (name == "Point")
	{
		LoadPoint(element);
	}
	else if (name == "Torus")
	{
		LoadTorus(element);
	}
	else if (name == "BezierC0")
	{
		LoadBezierC0(element);
	}
	else if (name == "BezierC2")
	{
		LoadBezierC2(element);
	}
	else if (name == "BezierInter")
	{
		LoadInterpolationSpline(element);
	}
	else if (name == "PatchC0")
	{
		LoadBezierC0Surface(element);
	}
	else if (name == "PatchC2")
	{
		LoadBezierC2Surface(element);
	}
}

bool SceneImporter::InvalidateFile(std::wstring path)
{
	return true;
}

std::string SceneImporter::GetName(tinyxml2::XMLElement* element)
{
	return std::string(element->Attribute("Name"));
}

Transform SceneImporter::GetTransform(tinyxml2::XMLElement* element)
{
	Transform t;
	t.SetPosition(GetFloat3Attribute(element, "Position"));
	t.SetRotation(GetFloat3Attribute(element, "Rotation"));
	t.SetScale(GetFloat3Attribute(element, "Scale"));

	return t;
}

DirectX::XMFLOAT3 SceneImporter::GetFloat3Attribute(tinyxml2::XMLElement* element, std::string attributeName)
{
	auto elem = element->FirstChildElement(attributeName.c_str());
	DirectX::XMFLOAT3 res;
	res.x = elem->FloatAttribute("X");
	res.y = elem->FloatAttribute("Y");
	res.z = elem->FloatAttribute("Z");

	return res;
}

std::vector<std::weak_ptr<Node>> SceneImporter::LoadPointReferences(tinyxml2::XMLElement* element)
{
	std::vector<std::weak_ptr<Node>> pointList;
	auto points = element->FirstChildElement("Points");
	auto point = points->FirstChildElement("PointRef");

	while (point != nullptr)
	{
		auto name = GetName(point);
		auto pointObj = std::find_if(m_loadedPoints.begin(), m_loadedPoints.end(),
			[name](std::shared_ptr<Node> pt) -> bool
			{
				return pt->m_object->m_name == name.c_str();
			});

		if (pointObj != m_loadedPoints.end())
		{
			pointList.push_back((*pointObj));
		}
		else {
			throw IncorrectFileFormatException();
		}

		point = point->NextSiblingElement();
	}

	return pointList;
}

std::vector<std::vector<std::weak_ptr<Node>>> SceneImporter::LoadGridPointReferences(tinyxml2::XMLElement* element)
{
	std::vector<std::vector<std::weak_ptr<Node>>> vertices;
	// REMEMBER TO REVERSE THE ROW ORDER (format requires row 0 at the top while this program uses row 0 at the bottom)
			
	std::vector<std::weak_ptr<Node>> pointList;
	auto points = element->FirstChildElement("Points");
	auto point = points->FirstChildElement("PointRef");
	// Iterate through all points and find the size of the grid

	int colCount = 0;
	int rowCount = 0;
	// Find row and column count
	while (point != nullptr)
	{
		int col = point->IntAttribute("Column");
		int row = point->IntAttribute("Row");

		colCount = max(col, colCount);
		rowCount = max(row, rowCount);
		point = point->NextSiblingElement();
	}
	colCount++;
	rowCount++;

	std::weak_ptr<Node>** objPoints = new std::weak_ptr<Node>*[colCount];

	for (int c = 0; c < colCount; c++)
	{
		objPoints[c] = new std::weak_ptr<Node>[rowCount];
	}

	point = points->FirstChildElement("PointRef");
	while (point != nullptr)
	{
		auto name = GetName(point);
		int col = point->IntAttribute("Column");
		int row = point->IntAttribute("Row");
		auto pointObj = std::find_if(m_loadedPoints.begin(), m_loadedPoints.end(),
			[name](std::shared_ptr<Node> pt) -> bool
			{
				return pt->m_object->m_name == name.c_str();
			});

		if (pointObj != m_loadedPoints.end())
		{
			objPoints[col][rowCount - row - 1] = *pointObj;
		}
		else {
			throw IncorrectFileFormatException();
			// ERROR - WRONG FORMAT
		}

		point = point->NextSiblingElement();
	}
	
	for (int col = 0; col < colCount; col++)
	{
		vertices.push_back(std::vector<std::weak_ptr<Node>>());
		for (int row = 0; row < rowCount; row++)
		{
			vertices[col].push_back(objPoints[col][row]);
		}
	}

	// release auxiliary structs
	for (int c = 0; c < colCount; c++)
	{
		delete[] objPoints[c];
	}
	delete[] objPoints;

	return vertices;
}

void SceneImporter::LoadPoint(tinyxml2::XMLElement* element)
{
	DirectX::XMFLOAT3 pos = GetFloat3Attribute(element, "Position");	
	auto pt = m_factory->CreatePoint();	
	pt->m_object->SetPosition(pos);
	// Add point to the loaded point list so that refs can be easily found
	pt->Rename(GetName(element));
	m_loadedPoints.push_back(pt);
	m_loadedObjects.push_back(pt);
}

void SceneImporter::LoadTorus(tinyxml2::XMLElement* element)
{
	auto name = GetName(element);
	Transform t = GetTransform(element);
	float majorRad = element->FloatAttribute("MajorRadius");
	float minorRad = element->FloatAttribute("MinorRadius");
	float xSlices = element->FloatAttribute("VerticalSlices");
	float ySlices = element->FloatAttribute("HorizontalSlices");
	
	auto torus = m_factory->CreateTorus(t, name, majorRad, minorRad, xSlices, ySlices);
	torus->Rename(GetName(element));
	m_loadedObjects.push_back(torus);	
}

void SceneImporter::LoadBezierC0(tinyxml2::XMLElement* element)
{
	bool showControlPolygon = element->BoolAttribute("ShowControlPolygon");

	// SET show control Polygon

	auto name = GetName(element);
	auto points = LoadPointReferences(element);

	auto curve = m_factory->CreateBezierCurve(points);
	curve->Rename(GetName(element));
	((BezierCurve*)curve->m_object.get())->SetDisplayPolygon(showControlPolygon);

	m_loadedObjects.push_back(curve);
}

void SceneImporter::LoadBezierC2(tinyxml2::XMLElement* element)
{
	bool showBernsteinPoints = element->BoolAttribute("ShowBernsteinPoints");
	bool showBernsteinPolygon = element->BoolAttribute("ShowBernsteinPolygon");
	bool showDeBoorPolygon = element->BoolAttribute("ShowDeBoorPolygon");

	auto name = GetName(element);
	auto points = LoadPointReferences(element);
	std::shared_ptr<Node> curve;
	if (showBernsteinPoints)
	{
		curve = m_factory->CreateBezierCurveC2(points,BezierBasis::Bernstein);
	}
	else {
		curve = m_factory->CreateBezierCurveC2(points);
	}
	curve->Rename(GetName(element));
	((BezierCurve*)curve->m_object.get())->SetDisplayPolygon(showBernsteinPoints);
	m_loadedObjects.push_back(curve);
}

void SceneImporter::LoadInterpolationSpline(tinyxml2::XMLElement* element)
{
	bool showControlPolygon = element->BoolAttribute("ShowControlPolygon");
	// SET show control Polygon

	auto name = GetName(element);
	auto points = LoadPointReferences(element);

	auto curve = m_factory->CreateInterpolBezierCurveC2(points);
	curve->Rename(GetName(element));
	((BezierCurve*)curve->m_object.get())->SetDisplayPolygon(showControlPolygon);
	m_loadedObjects.push_back(curve);
}

void SceneImporter::LoadBezierC0Surface(tinyxml2::XMLElement* element)
{
	auto name = GetName(element);
	
	SurfaceWrapDirection wrapDir = SurfaceWrapDirection::None;
	bool showControlPolygon = element->BoolAttribute("ShowControlPolygon");
	float widthSlices = element->FloatAttribute("RowSlices");
	float heightSlices = element->FloatAttribute("ColumnSlices");
	auto wrapDirection = element->Attribute("WrapDirection");

	// SET show control Polygon
	// Set Slices 

	auto points = LoadGridPointReferences(element);

	int width = (points.size() - 1) / 3;
	int height = (points[0].size() - 1) / 3;

	if (strcmp(wrapDirection, "Column") == 0)
	{
		wrapDir = SurfaceWrapDirection::Width;
		width++;
	}
	if (strcmp(wrapDirection, "Row") == 0)
	{
		wrapDir = SurfaceWrapDirection::Height;
		height++;
	}
	if (strcmp(wrapDirection, "None") == 0)
	{
		wrapDir = SurfaceWrapDirection::None;
	}

	auto surface = m_factory->CreateBezierSurface(points,width, height, wrapDir);
	surface->Rename(GetName(element));
	((BezierSurfaceC0*)surface->m_object.get())->SetDivisions(widthSlices+1, heightSlices+1);
	((BezierSurfaceC0*)surface->m_object.get())->SetDisplayPolygon(showControlPolygon);
	m_loadedObjects.push_back(surface);
}

void SceneImporter::LoadBezierC2Surface(tinyxml2::XMLElement* element)
{
	SurfaceWrapDirection wrapDir = SurfaceWrapDirection::None;
	bool showControlPolygon = element->BoolAttribute("ShowControlPolygon");
	float widthSlices = element->FloatAttribute("RowSlices");
	float heightSlices = element->FloatAttribute("ColumnSlices");
	auto wrapDirection = element->Attribute("WrapDirection");

	// SET show control Polygon
	// Set Slices 

	auto points = LoadGridPointReferences(element);

	int width = (points.size() - 3);
	int height = (points[0].size() - 3);

	if (strcmp(wrapDirection, "Column") == 0)
	{
		wrapDir = SurfaceWrapDirection::Width;
		width+=3;
	}
	if (strcmp(wrapDirection, "Row") == 0)
	{
		wrapDir = SurfaceWrapDirection::Height;
		height+=3;
	}
	if (strcmp(wrapDirection, "None") == 0)
	{
		wrapDir = SurfaceWrapDirection::None;
	}

	auto surface = m_factory->CreateBezierSurfaceC2(points, width, height, wrapDir);
	surface->Rename(GetName(element));
	((BezierSurfaceC0*)surface->m_object.get())->SetDivisions(widthSlices + 1, heightSlices + 1);
	((BezierSurfaceC0*)surface->m_object.get())->SetDisplayPolygon(showControlPolygon);
	m_loadedObjects.push_back(surface);
}

void SceneImporter::SwitchToLoadedScene()
{

	for (int i = 0; i < m_loadedObjects.size(); i++)
	{
		m_scene->AttachObject(m_loadedObjects[i]);
	}
}
