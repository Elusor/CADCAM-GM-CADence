#include "BezierCurve.h"
#include "Node.h"
#include "imgui.h"
//#include "imgui.cpp"
#include "Scene.h"
BezierCurve::BezierCurve(std::vector<std::weak_ptr<Node>> initialControlPoints)
{
	m_renderPolygon = false;
	m_controlPoints = initialControlPoints;
}

void BezierCurve::AttachChild(std::weak_ptr<Node> controlPoint)
{
	m_controlPoints.push_back(controlPoint);
}

void BezierCurve::RemoveChild(std::weak_ptr<Node> controlPoint)
{	
	if (auto controlPt = controlPoint.lock())
	{
		auto m_copy = m_controlPoints;
		auto it = m_copy.begin();
		while (it != m_copy.end())
		{
			if (auto node = it->lock())
			{
				if (node == controlPt)
				{
					it = m_copy.erase(it);
				}
				else {
					it++;
				}
			}
			else {
				it = m_copy.erase(it);
			}			
		}
		m_controlPoints = m_copy;
	}	
}

bool BezierCurve::IsChild(std::weak_ptr<Node> point)
{
	if (std::shared_ptr<Node> candidate = point.lock())
	{
		for (int i = 0; i < m_controlPoints.size(); i++)
		{
			if (candidate == m_controlPoints[i].lock())
				return true;
		}
	}
	return false;
}

void BezierCurve::RenderObjectSpecificContextOptions(Scene& scene)
{
	// Get points that do not belong to this point
	std::vector<Point*> points;

	for (int i = 0; i < scene.m_nodes.size(); i++)
	{
		auto currentobject = (scene.m_nodes[i]->m_object.get());
		if (typeid(*currentobject) == typeid(Point))
		{
			Point* point = dynamic_cast<Point*>(currentobject);
			if (IsChild(point->m_parent) == false)
			{
				points.push_back(point);
			}
		}
	}

	// If there are any - display them in the context menu
	if (points.size() > 0)
	{
		if (ImGui::BeginMenu("Add to Curve"))
		{
			if (auto parent = (m_parent.lock()))
			{
				auto gParent = dynamic_cast<GroupNode*>(parent.get());
				for (int i = 0; i < points.size(); i++)
				{			
					if (auto node = points[i]->m_parent.lock())
					{
						if (ImGui::MenuItem(node->GetLabel().c_str()))
						{
							gParent->AddChild(node);
							AttachChild(node);
						}
					}								
				}
			}
			ImGui::EndMenu();
		}
	}
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
