#pragma once
#include "Scene.h"
#include "objects.h"
#include "tinyxml2.h"

class SceneExporter {
public:
	SceneExporter(Scene* scene);
	bool Export(std::wstring path);

private:
	Scene* m_scene;

	void ExportObject(std::shared_ptr<Node> object);
	void ExportPoint(std::shared_ptr<Node> object);
	void ExportTorus(std::shared_ptr<Node> object);
	void ExportCurveC0(std::shared_ptr<Node> object);
	void ExportCurveC2(std::shared_ptr<Node> object);
	void ExportInterpolCurve(std::shared_ptr<Node> object);
	void ExportSurfaceC0(std::shared_ptr<Node> object);
	void ExportSurfaceC2(std::shared_ptr<Node> object);
};