#pragma once
#include <string>
#include "Scene.h"
#include "GuiManager.h"
#include "ObjectFactory.h"
#include "tinyxml2.h"
#include "objects.h"
class SceneImporter
{
public:
	SceneImporter(Scene* scene, GuiManager* guiManager);
	bool Import(std::wstring path);
	bool InvalidateFile(std::wstring path);
private:
	Scene* m_scene;
	ObjectFactory* m_factory;
	GuiManager* m_guiManager;

	std::vector<std::shared_ptr<Node>> m_loadedPoints;
	std::vector<std::shared_ptr<Node>> m_loadedObjects;

	void ProcessElement(tinyxml2::XMLElement* element);

	std::string GetName(tinyxml2::XMLElement* element);
	Transform GetTransform(tinyxml2::XMLElement* element);
	DirectX::XMFLOAT3 GetFloat3Attribute(tinyxml2::XMLElement* element, std::string attributeName);

	std::vector<std::weak_ptr<Node>> LoadPointReferences(tinyxml2::XMLElement* element);
	std::vector<std::vector<std::weak_ptr<Node>>> LoadGridPointReferences(tinyxml2::XMLElement* element);

	void LoadPoint(tinyxml2::XMLElement* element);
	void LoadTorus(tinyxml2::XMLElement* element);
	void LoadBezierC0(tinyxml2::XMLElement* element);
	void LoadBezierC2(tinyxml2::XMLElement* element);
	void LoadInterpolationSpline(tinyxml2::XMLElement* element);
	void LoadBezierC0Surface(tinyxml2::XMLElement* element);
	void LoadBezierC2Surface(tinyxml2::XMLElement* element);
};