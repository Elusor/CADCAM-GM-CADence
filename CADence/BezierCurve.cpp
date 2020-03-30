#include "BezierCurve.h"
#include "bezierCalculator.h"
#include "Node.h"
#include "imgui.h"
//#include "imgui.cpp"
#include "adaptiveRenderingCalculator.h"
#include "Scene.h"

BezierCurve::BezierCurve(): BezierCurve(std::vector<std::weak_ptr<Node>>())
{
	m_renderPolygon = false;

}

BezierCurve::BezierCurve(std::vector<std::weak_ptr<Node>> initialControlPoints)
{
	m_modified = true;
	m_adaptiveRenderingSamples = 0;
	m_renderPolygon = false;
	m_controlPoints = initialControlPoints;
}

void BezierCurve::AttachChild(std::weak_ptr<Node> controlPoint)
{
	m_controlPoints.push_back(controlPoint);
	SetModified(true);
}

void BezierCurve::RemoveChild(std::weak_ptr<Node> controlPoint)
{	
	if (auto controlPt = controlPoint.lock())
	{
		auto it = m_controlPoints.begin();
		while (it != m_controlPoints.end())
		{
			if (auto node = it->lock())
			{
				if (node == controlPt)
				{
					it = m_controlPoints.erase(it);
				}
				else {
					it++;
				}
			}
			else {
				it = m_controlPoints.erase(it);
			}			
		}
	}	
	SetModified(true);
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
	RemoveExpiredChildren();
	if (m_controlPoints.size() > 1)
	{		
		int prev = m_adaptiveRenderingSamples;
		m_adaptiveRenderingSamples = AdaptiveRenderingCalculator::CalculateAdaptiveSamplesCount(m_controlPoints, renderData);
		if (prev != m_adaptiveRenderingSamples)
			SetModified(true);

		if (m_modified)
		{
			UpdateObject();
		}

		if (m_controlPoints.size() == 4)
		{
			renderData->m_device.context()->GSSetShader(
				renderData->m_bezierGeometryShader.get(),
				nullptr, 0);

			std::vector<VertexPositionColor> vertices;
			std::vector<unsigned short> indices;

			for (int i = 0; i < 4; i++)
			{
				if (auto point1 = m_controlPoints[i].lock())
				{
					vertices.push_back(VertexPositionColor{
							point1->m_object->GetPosition(),
							m_meshDesc.m_defaultColor });
				}
			}

			m_meshDesc.vertices = vertices;
			m_meshDesc.indices = {0,1,2,3};
			m_meshDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
		}

		MeshObject::RenderObject(renderData);

		renderData->m_device.context()->GSSetShader(nullptr, nullptr, 0);

		if (m_renderPolygon)
		{
			RenderMesh(renderData, m_PolygonDesc);
		}
	}	
}

void BezierCurve::RemoveExpiredChildren()
{
	auto it = m_controlPoints.begin();
	while (it != m_controlPoints.end())
	{
		if (auto pt = it->lock())
		{
			it++;
		}
		else {
			it = m_controlPoints.erase(it);
		}		
	}

	if (auto parent = m_parent.lock())
	{
		auto gParent = dynamic_cast<GroupNode*>(parent.get());
		gParent->RemoveExpiredChildren();
	}
}

bool BezierCurve::CreateParamsGui()
{	
	ImGui::Begin("Inspector");
	// Create sliders for torus parameters	
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());
	ImGui::Text("Samples: ");
	ImGui::SameLine(); ImGui::Text(std::to_string(m_adaptiveRenderingSamples).c_str());

	
	ImGui::Spacing();	
	bool objectChanged = false;	

	// checkbox for deBoore points

	// change colors for the curve
	float mcolor[3] = {
		m_meshDesc.m_defaultColor.x,
		m_meshDesc.m_defaultColor.y,
		m_meshDesc.m_defaultColor.z,
	};

	std::string mtext = "Curve color";
	ImGui::Text(mtext.c_str());
	objectChanged |= ImGui::ColorEdit3(("##" + mtext + GetIdentifier()).c_str(), (float*)&mcolor);

	m_meshDesc.m_defaultColor.x = mcolor[0];
	m_meshDesc.m_defaultColor.y = mcolor[1];
	m_meshDesc.m_defaultColor.z = mcolor[2];
	ImGui::Spacing();

	// checkbox for Bernstein's polygon
	std::string label = "Display Bernstein polygon" + GetIdentifier();
	objectChanged |= ImGui::Checkbox(label.c_str(), &m_renderPolygon);
	ImGui::Spacing();

	// change colors for polygon
	float pcolor[3] = {
		m_PolygonDesc.m_defaultColor.x,
		m_PolygonDesc.m_defaultColor.y,
		m_PolygonDesc.m_defaultColor.z,
	};

	std::string ptext = "Bernstein Polygon color";
	ImGui::Text(ptext.c_str());
	objectChanged |= ImGui::ColorEdit3(("##" + ptext + GetIdentifier()).c_str(), (float*)&pcolor);

	m_PolygonDesc.m_defaultColor.x = pcolor[0];
	m_PolygonDesc.m_defaultColor.y = pcolor[1];
	m_PolygonDesc.m_defaultColor.z = pcolor[2];
	ImGui::Spacing();

	

	ImGui::End();
	return objectChanged;
}

bool BezierCurve::GetIsModified()
{
	for (int i = 0; i < m_controlPoints.size(); i++)
	{
		if (auto point = m_controlPoints[i].lock())
		{
			if (point->m_object->GetIsModified())
			{
				SetModified(true);
			}
		}
	}

	return m_modified;
}

void BezierCurve::UpdateObject()
{
	if (m_controlPoints.size() > 0)
	{
		// Render object using De Casteljau algorithm
		std::vector<DirectX::XMFLOAT3> knots;

		std::vector<VertexPositionColor> polygonVertices;
		std::vector<unsigned short> polygonIndices;

		for (int i = 0; i < m_controlPoints.size(); i++)
		{
			if (auto point = m_controlPoints[i].lock())
			{
				knots.push_back(point->m_object->GetPosition());
			}

			//Update Bezier Polygon
			polygonVertices.push_back(VertexPositionColor{
				knots[i],
				 m_PolygonDesc.m_defaultColor
				});
			polygonIndices.push_back(i);
		}

		// Update bezier polygon vertex description
		m_PolygonDesc.vertices = polygonVertices;
		m_PolygonDesc.indices = polygonIndices;
		m_PolygonDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

		// Get Bezier Curve Points
		auto points = BezierCalculator::CalculateBezierDeCasteljau(knots, m_adaptiveRenderingSamples);

		std::vector<VertexPositionColor> vertices;
		std::vector<unsigned short> indices;

		for (int i = 0; i < points.size(); i++)
		{
			vertices.push_back(VertexPositionColor{
				points[i],
				m_meshDesc.m_defaultColor
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