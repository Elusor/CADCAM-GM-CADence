#include "BezierCurve.h"
#include "bezierCalculator.h"
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

void BezierCurve::RenderObject(std::unique_ptr<RenderState>& renderData)
{
	// TODO [MG] DO NOT RECALCULATE EACH FRAME
	if (m_controlPoints.size() > 0)
	{
		UpdateObject();
		MeshObject::RenderObject(renderData);	
	}	
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
	int adaptiveRenderingSamples = 200;

	if (m_controlPoints.size() > 0)
	{
		// Render object using De Casteljau algorithm
		std::vector<Transform> knots;

		for (int i = 0; i < m_controlPoints.size(); i++)
		{
			if (auto point = m_controlPoints[i].lock())
			{
				knots.push_back(point->m_object->GetTransform());
			}
		}

		// Get Bezier Curve Points
		auto points = BezierCalculator::CalculateBezierC0Values(knots, adaptiveRenderingSamples);

		std::vector<VertexPositionColor> vertices;
		std::vector<unsigned short> indices;

		for (int i = 0; i < points.size(); i++)
		{
			vertices.push_back(VertexPositionColor{
				points[i].GetPosition(),
				{1.0f,1.0f,1.0f}
				});
			indices.push_back(i);
		}

		m_meshDesc.vertices = vertices;
		m_meshDesc.indices = indices;
		m_meshDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	}
	// Recalculate adaptive rendering??	
}

#pragma region Transform Wrappers
void BezierCurve::SetTransform(Transform transform)
{
	// Perhaps change transform type instead of overriding SetTransform method
	// Ignore 
}

void BezierCurve::SetPosition(DirectX::XMFLOAT3 position)
{
	// Ignore 
}

void BezierCurve::SetRotation(DirectX::XMFLOAT3 rotation)
{
	// Ignore 
}

void BezierCurve::SetScale(DirectX::XMFLOAT3 scale)
{
	// Ignore 
}
void BezierCurve::Translate(DirectX::XMFLOAT3 position)
{
	// Ignore 

}
void BezierCurve::Rotate(DirectX::XMFLOAT3 rotation)
{
	// Ignore 

}
void BezierCurve::Scale(DirectX::XMFLOAT3 scale)
{
	// Ignore 
}
#pragma endregion