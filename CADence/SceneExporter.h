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

	void ExportObject(tinyxml2::XMLElement* scene, std::shared_ptr<Node> object);
	void ExportPoint(tinyxml2::XMLElement* scene, Point* point);
	void ExportTorus(tinyxml2::XMLElement* scene, Torus* torus);
	void ExportCurveC0(tinyxml2::XMLElement* scene, BezierCurve* curve);
	void ExportCurveC2(tinyxml2::XMLElement* scene, BezierCurveC2* curve);
	void ExportInterpolCurve(tinyxml2::XMLElement* scene, InterpolationBezierCurveC2* curve);
	void ExportSurfaceC0(tinyxml2::XMLElement* scene, BezierSurfaceC0* surface);
	void ExportSurfaceC2(tinyxml2::XMLElement* scene, BezierSurfaceC2* surface);

	void ExportCurvePoints(tinyxml2::XMLElement* object, BezierCurve* curve);
	void ExportGridPoints(tinyxml2::XMLElement* object, BezierSurfaceC0* curve);
};