#include "SceneExporter.h"
#include <codecvt>

SceneExporter::SceneExporter(Scene* scene)
{
	m_scene = scene;
}

bool SceneExporter::Export(std::wstring path)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;	
	m_scene->ClearScene();
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
		ExportObject(*obj);
	}

	doc.LinkEndChild(scene);

	auto res = doc.SaveFile(pathStr.c_str());
	return res == tinyxml2::XMLError::XML_SUCCESS;
}

void SceneExporter::ExportObject(std::shared_ptr<Node> object)
{

}
