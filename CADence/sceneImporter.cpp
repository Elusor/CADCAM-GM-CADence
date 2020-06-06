#include "sceneImporter.h"
#include <cassert>
#include <codecvt>

SceneImporter::SceneImporter(Scene* scene)
{
	m_scene = scene;
	m_factory = scene->m_objectFactory.get();
}

bool SceneImporter::Import(std::wstring wpath)
{
	// TODO: check if file is correct
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;

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

std::string SceneImporter::GetName(tinyxml2::XMLElement* element)
{
	return std::string(element->Name());
}

DirectX::XMFLOAT3 SceneImporter::GetPosition(tinyxml2::XMLElement* element)
{
	auto position = element->FirstChildElement("Position");
	DirectX::XMFLOAT3 pos;
	pos.x = position->FloatAttribute("X");
	pos.y = position->FloatAttribute("Y");
	pos.z = position->FloatAttribute("Z");

	return pos;
}

void SceneImporter::LoadPointReferences(tinyxml2::XMLElement* element)
{
}

void SceneImporter::LoadPoint(tinyxml2::XMLElement* element)
{
	auto name = GetName(element);	
	DirectX::XMFLOAT3 pos = GetPosition(element);	
	auto pt = m_factory->CreatePoint();
	m_scene->AttachObject(pt);
	pt->m_object->SetPosition(pos);
}

void SceneImporter::LoadTorus(tinyxml2::XMLElement* element)
{
	auto name = GetName(element);

}

void SceneImporter::LoadBezierC0(tinyxml2::XMLElement* element)
{
	auto name = GetName(element);

}

void SceneImporter::LoadBezierC2(tinyxml2::XMLElement* element)
{
	auto name = GetName(element);

}

void SceneImporter::LoadInterpolationSpline(tinyxml2::XMLElement* element)
{
	auto name = GetName(element);

}

void SceneImporter::LoadBezierC0Surface(tinyxml2::XMLElement* element)
{
	auto name = GetName(element);

}

void SceneImporter::LoadBezierC2Surface(tinyxml2::XMLElement* element)
{
	auto name = GetName(element);

}
