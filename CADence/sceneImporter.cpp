#include "sceneImporter.h"
#include <cassert>
#include <codecvt>
#include <algorithm>

SceneImporter::SceneImporter(Scene* scene)
{
	m_scene = scene;
	m_factory = scene->m_objectFactory.get();
}

bool SceneImporter::Import(std::wstring wpath)
{
	// TODO: check if file is correct
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
	
	m_loadedPoints.clear();
	m_scene->ClearScene();
	auto path = converterX.to_bytes(wpath);

	tinyxml2::XMLDocument doc;
	auto res = doc.LoadFile((path).c_str());
	if (res != tinyxml2::XMLError::XML_SUCCESS)
		return false;
	
	tinyxml2::XMLElement* scene = doc.FirstChildElement("Scene");
	tinyxml2::XMLElement* element = scene->FirstChildElement();

	while (element != nullptr)
	{
		ProcessElement(element);
		element = element->NextSiblingElement();
	}
	m_loadedPoints.clear();
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
		auto name = point->Attribute("Name");		
		auto pointObj = std::find_if(m_loadedPoints.begin(), m_loadedPoints.end(),
			[name](std::shared_ptr<Node> pt) -> bool
			{
				return pt->m_object->m_name == name;
			});

		if (pointObj != m_loadedPoints.end())
		{
			pointList.push_back((*pointObj));
		}
		else {
			// ERROR - WRONG FORMAT
		}

		point = point->NextSiblingElement();
	}

	return pointList;
}

void SceneImporter::LoadPoint(tinyxml2::XMLElement* element)
{
	DirectX::XMFLOAT3 pos = GetFloat3Attribute(element, "Position");	
	auto pt = m_factory->CreatePoint();
	m_scene->AttachObject(pt);
	pt->m_object->SetPosition(pos);
	// Add point to the loaded point list so that refs can be easily found
	pt->Rename(GetName(element));
	m_loadedPoints.push_back(pt);
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
	m_scene->AttachObject(torus);
}

void SceneImporter::LoadBezierC0(tinyxml2::XMLElement* element)
{
	bool showControlPolygon = element->BoolAttribute("ShowControlPolygon");
	auto name = GetName(element);
	auto points = LoadPointReferences(element);

	auto curve = m_factory->CreateBezierCurve(points);
	curve->Rename(GetName(element));
	m_scene->AttachObject(curve);	
}

void SceneImporter::LoadBezierC2(tinyxml2::XMLElement* element)
{
	bool showBernsteinPoints = element->BoolAttribute("ShowBernsteinPoints");
	bool showBernsteinPolygon = element->BoolAttribute("ShowBernsteinPolygon");
	bool showDeBoorPolygon = element->BoolAttribute("ShowDeBoorPolygon");
	auto name = GetName(element);
	auto points = LoadPointReferences(element);

	auto curve = m_factory->CreateBezierCurveC2(points);
	curve->Rename(GetName(element));
	m_scene->AttachObject(curve);
}

void SceneImporter::LoadInterpolationSpline(tinyxml2::XMLElement* element)
{
	bool showControlPolygon = element->BoolAttribute("ShowControlPolygon");
	auto name = GetName(element);
	auto points = LoadPointReferences(element);

	auto curve = m_factory->CreateInterpolBezierCurveC2(points);
	curve->Rename(GetName(element));
	m_scene->AttachObject(curve);
}

void SceneImporter::LoadBezierC0Surface(tinyxml2::XMLElement* element)
{
	auto name = GetName(element);
	
	SurfaceWrapDirection wrapDir = SurfaceWrapDirection::None;
	bool showControlPolygon = element->BoolAttribute("ShowControlPolygon");
	float widthSlices = element->FloatAttribute("RowSlices");
	float heightSlices = element->FloatAttribute("ColumnSlices");
	auto wrapDirection = element->Attribute("WrapDirection");
	
	if (wrapDirection == "Column")
	{
		wrapDir = SurfaceWrapDirection::Width;
	}
	if (wrapDirection == "Row")
	{
		wrapDir = SurfaceWrapDirection::Height;
	}
	if (wrapDirection == "None")
	{
		wrapDir = SurfaceWrapDirection::None;
	}

	//surface->Rename(GetName(element));
	LoadPointReferences(element);
}

void SceneImporter::LoadBezierC2Surface(tinyxml2::XMLElement* element)
{
	auto name = GetName(element);

	SurfaceWrapDirection wrapDir = SurfaceWrapDirection::None;
	bool showControlPolygon = element->BoolAttribute("ShowControlPolygon");
	float widthSlices = element->FloatAttribute("RowSlices");
	float heightSlices = element->FloatAttribute("ColumnSlices");
	auto wrapDirection = element->Attribute("WrapDirection");

	if (wrapDirection == "Column")
	{
		wrapDir = SurfaceWrapDirection::Width;
	}
	if (wrapDirection == "Row")
	{
		wrapDir = SurfaceWrapDirection::Height;
	}
	if (wrapDirection == "None")
	{
		wrapDir = SurfaceWrapDirection::None;
	}

	//surface->Rename(GetName(element));
	LoadPointReferences(element);
}
