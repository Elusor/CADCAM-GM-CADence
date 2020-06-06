#pragma once
#include <string>
#include "Scene.h"
#include "ObjectFactory.h"
#include "tinyxml2.h"

class SceneImporter
{
public:
	SceneImporter(Scene* scene);
	bool Import(std::wstring path);
private:
	Scene* m_scene;
	ObjectFactory* m_factory;

	void ProcessElement(tinyxml2::XMLElement* element);

	std::string GetName(tinyxml2::XMLElement* element);
	DirectX::XMFLOAT3 GetPosition(tinyxml2::XMLElement* element);

	void LoadPointReferences(tinyxml2::XMLElement* element);
	void LoadPoint(tinyxml2::XMLElement* element);
	void LoadTorus(tinyxml2::XMLElement* element);
	void LoadBezierC0(tinyxml2::XMLElement* element);
	void LoadBezierC2(tinyxml2::XMLElement* element);
	void LoadInterpolationSpline(tinyxml2::XMLElement* element);
	void LoadBezierC0Surface(tinyxml2::XMLElement* element);
	void LoadBezierC2Surface(tinyxml2::XMLElement* element);
};