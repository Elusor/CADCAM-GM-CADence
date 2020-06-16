#include "SceneExporter.h"
#include <codecvt>
#include <algorithm>

SceneExporter::SceneExporter(Scene* scene, GuiManager* guiManager)
{
	m_scene = scene;
	m_guiManager = guiManager;
}

bool SceneExporter::Export(std::wstring path)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;	
	auto pathStr = converterX.to_bytes(path);
	tinyxml2::XMLDocument doc;

	auto declaration = doc.NewDeclaration();
	doc.LinkEndChild(declaration);

	auto scene = doc.NewElement("Scene");
	scene->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	scene->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	scene->SetAttribute("xmlns", "http://mini.pw.edu.pl/mg1");

	auto objects = m_scene->m_nodes;
	for (auto obj = objects.begin(); obj != objects.end(); obj++)
	{
		ExportObject(scene, *obj);
	}

	doc.LinkEndChild(scene);

	auto res = doc.SaveFile(pathStr.c_str());
	return res == tinyxml2::XMLError::XML_SUCCESS;
}

bool SceneExporter::InvalidateScene()
{
	return CheckDuplicateNames();
}

bool SceneExporter::CheckDuplicateNames()
{
	bool duplicate = false;

	auto objects = m_scene->m_nodes;
	for (auto it = objects.begin(); it != objects.end(); it++)
	{
		auto obj = (*it)->m_object.get();
		auto dup = std::find_if(objects.begin(), objects.end(), [obj](std::shared_ptr<Node> pt) -> bool
			{
				return (pt->m_object->m_name == obj->m_name.c_str()) && (pt->m_object.get() != obj);
			});
		if (dup!= objects.end())
		{
			duplicate = true;
		}
	}

	if (duplicate)
	{
		m_guiManager->EnableCustomModal("Scene invalid. Detected duplicate object names.", "Scene invalidation error");
	}

	return !duplicate;
}

void SceneExporter::ExportObject(tinyxml2::XMLElement* scene, std::shared_ptr<Node> object)
{
	auto* obj = object->m_object.get();
	
	if (typeid(*obj) == typeid(Point))
	{
		ExportPoint(scene,(Point*)obj);
	} 
	else if (typeid(*obj) == typeid(Torus))
	{
		ExportTorus(scene, (Torus*)obj);
	} 
	else if (typeid(*obj) == typeid(InterpolationBezierCurveC2))
	{
		ExportInterpolCurve(scene, (InterpolationBezierCurveC2*)obj);
	}
	else if (typeid(*obj) == typeid(BezierCurveC2))
	{
		ExportCurveC2(scene, (BezierCurveC2*)obj);
	}
	else if (typeid(*obj) == typeid(BezierCurve))
	{
		ExportCurveC0(scene, (BezierCurve*)obj);
	}
	else if (typeid(*obj) == typeid(BezierSurfaceC2))
	{
		ExportSurfaceC2(scene, (BezierSurfaceC2*)obj);
	}
	else if (typeid(*obj) == typeid(BezierSurfaceC0))
	{
		ExportSurfaceC0(scene, (BezierSurfaceC0*)obj);
	}

}

void SceneExporter::ExportPoint(tinyxml2::XMLElement* scene, Point* point)
{
	auto object = scene->InsertNewChildElement("Point");
	object->SetAttribute("Name", point->m_name.c_str());
	auto postion = object->InsertNewChildElement("Position");
	XMFLOAT3 pos = point->GetPosition();
	postion->SetAttribute("X", pos.x);
	postion->SetAttribute("Y", pos.y);
	postion->SetAttribute("Z", pos.z);
}

void SceneExporter::ExportTorus(tinyxml2::XMLElement* scene, Torus* torus)
{
	auto object = scene->InsertNewChildElement("Torus");
	object->SetAttribute("Name", torus->m_name.c_str());
	object->SetAttribute("MajorRadius", torus->m_bigR);
	object->SetAttribute("MinorRadius", torus->m_smallR);
	object->SetAttribute("VerticalSlices", torus->m_surParams.densityX);
	object->SetAttribute("HorizontalSlices", torus->m_surParams.densityY);

	auto postion = object->InsertNewChildElement("Position");
	XMFLOAT3 pos = torus->GetPosition();
	postion->SetAttribute("X", pos.x);
	postion->SetAttribute("Y", pos.y);
	postion->SetAttribute("Z", pos.z);

	auto rotation = object->InsertNewChildElement("Rotation");
	XMFLOAT3 rot = torus->GetRotation();
	rotation->SetAttribute("X", rot.x);
	rotation->SetAttribute("Y", rot.y);
	rotation->SetAttribute("Z", rot.z);

	auto scale = object->InsertNewChildElement("Scale");
	XMFLOAT3 scal = torus->GetScale();
	scale->SetAttribute("X", scal.x);
	scale->SetAttribute("Y", scal.y);
	scale->SetAttribute("Z", scal.z);	
}

void SceneExporter::ExportCurveC0(tinyxml2::XMLElement* scene, BezierCurve* curve)
{
	auto object = scene->InsertNewChildElement("BezierC0");
	object->SetAttribute("Name", curve->m_name.c_str());
	object->SetAttribute("ShowControlPolygon", curve->GetDisplayPolygon());
	ExportCurvePoints(object, curve);
}

void SceneExporter::ExportCurveC2(tinyxml2::XMLElement* scene, BezierCurveC2* curve)
{
	auto object = scene->InsertNewChildElement("BezierC2");
	object->SetAttribute("Name", curve->m_name.c_str());
	object->SetAttribute("ShowBernsteinPoints", curve->GetCurrentBasis() == BezierBasis::Bernstein);
	object->SetAttribute("ShowBernsteinPolygon", curve->GetDisplayPolygon());
	object->SetAttribute("ShowDeBoorPolygon", curve->GetDisplayDeBoorPolygon());
	ExportCurvePoints(object, curve);
}

void SceneExporter::ExportInterpolCurve(tinyxml2::XMLElement* scene, InterpolationBezierCurveC2* curve)
{
	auto object = scene->InsertNewChildElement("BezierInter");
	object->SetAttribute("Name", curve->m_name.c_str());
	object->SetAttribute("ShowControlPolygon", curve->GetDisplayPolygon());
	ExportCurvePoints(object, curve);
}

void SceneExporter::ExportSurfaceC0(tinyxml2::XMLElement* scene, BezierSurfaceC0* surface)
{
	auto object = scene->InsertNewChildElement("PatchC0");
	object->SetAttribute("Name", surface->m_name.c_str());
	object->SetAttribute("ShowControlPolygon", surface->GetDisplayPolygon());

	switch (surface->GetWrapDirection())
	{
		case SurfaceWrapDirection::Height:
			object->SetAttribute("WrapDirection", "Row");
			break;	
		case SurfaceWrapDirection::Width:
			object->SetAttribute("WrapDirection", "Column");
			break;
		case SurfaceWrapDirection::None:
			object->SetAttribute("WrapDirection", "None");
			break;
	}
	
	object->SetAttribute("RowSlices", surface->GetUDivisions() - 1);
	object->SetAttribute("ColumnSlices", surface->GetVDivisions() - 1);
	ExportGridPoints(object, surface);
}

void SceneExporter::ExportSurfaceC2(tinyxml2::XMLElement* scene, BezierSurfaceC2* surface)
{
	auto object = scene->InsertNewChildElement("PatchC2");
	object->SetAttribute("Name", surface->m_name.c_str());
	object->SetAttribute("ShowControlPolygon", surface->GetDisplayPolygon());

	switch (surface->GetWrapDirection())
	{
	case SurfaceWrapDirection::Height:
		object->SetAttribute("WrapDirection", "Row");
		break;
	case SurfaceWrapDirection::Width:
		object->SetAttribute("WrapDirection", "Column");
		break;
	case SurfaceWrapDirection::None:
		object->SetAttribute("WrapDirection", "None");
		break;
	}

	object->SetAttribute("RowSlices", surface->GetUDivisions() - 1);
	object->SetAttribute("ColumnSlices", surface->GetVDivisions() - 1);
	ExportGridPoints(object, surface);
}

void SceneExporter::ExportCurvePoints(tinyxml2::XMLElement* object, BezierCurve* curve)
{
	auto list = object->InsertNewChildElement("Points");
	auto points = curve->GetControlPoints();

	for (auto pt = points.begin(); pt != points.end(); pt++)
	{
		auto ptObj = pt->lock();
		auto pointRef = list->InsertNewChildElement("PointRef");
		pointRef->SetAttribute("Name", ptObj->m_object->m_name.c_str());
	}
}

void SceneExporter::ExportGridPoints(tinyxml2::XMLElement* object, BezierSurfaceC0* surface)
{
	auto list = object->InsertNewChildElement("Points");
	int width; 
	int height;
	auto points = surface->GetPoints(height, width);
	
	
	for (int h = height-1; h >= 0; h--)
	{
		for (int w = 0; w < width; w++)
		{
		auto ptObj = points[h + w * height].lock();
		auto pointRef = list->InsertNewChildElement("PointRef");
		pointRef->SetAttribute("Name", ptObj->m_object->m_name.c_str());
		pointRef->SetAttribute("Row", height -1 - h);
		pointRef->SetAttribute("Column", w);
		}
	}
	
}
