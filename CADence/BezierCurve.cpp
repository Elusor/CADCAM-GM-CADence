#include "BezierCurve.h"
#include "Node.h"
#include "imgui.h"
//#include "imgui.cpp"

BezierCurve::BezierCurve(std::vector<std::weak_ptr<Node>> initialControlPoints)
{
	m_renderPolygon = false;
	m_controlPoints = initialControlPoints;
}

void BezierCurve::AttachChild(std::weak_ptr<Node> controlPoint)
{
	m_controlPoints.push_back(controlPoint);
}

void BezierCurve::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	// Render object using De Casteljau algorithm
}

bool BezierCurve::CreateParamsGui()
{
	
	ImGui::Begin("Inspector");
	// Create sliders for torus parameters	
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());
	ImGui::Spacing();	
	bool objectChanged = false;	
	// checkbox for Bernstein's polygon
	// checkbox for deBoore points
	ImGui::End();
	return objectChanged;
}

void BezierCurve::UpdateObject()
{
	// Recalculate adaptive rendering??
}
