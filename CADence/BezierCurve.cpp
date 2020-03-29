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
	// TODO [MG] DO NOT RECALCULATE EACH FRAME
	if (m_controlPoints.size() > 1)
	{
		m_adaptiveRenderingSamples = AdaptiveRenderingCalculator::CalculateAdaptiveSamplesCount(m_controlPoints, renderData);
		UpdateObject();
		MeshObject::RenderObject(renderData);	
		if (m_renderPolygon)
		{	
			RenderMesh(renderData, m_PolygonDesc);
		}
		
	}	
}

//void BezierCurve::RenderPolygon(std::unique_ptr<RenderState>& renderState)
//{
//	renderState->m_device.context()->IASetPrimitiveTopology(m_PolygonDesc.m_primitiveTopology);
//	//Update content to fill constant buffer
//	D3D11_MAPPED_SUBRESOURCE res;
//	DirectX::XMMATRIX mvp = m_transform.GetModelMatrix() * renderState->m_camera->GetViewProjectionMatrix();
//	//Set constant buffer
//	auto hres = renderState->m_device.context()->Map((renderState->m_cbMVP.get()), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
//	memcpy(res.pData, &mvp, sizeof(DirectX::XMMATRIX));
//	renderState->m_device.context()->Unmap(renderState->m_cbMVP.get(), 0);
//	ID3D11Buffer* cbs[] = { renderState->m_cbMVP.get() };
//	renderState->m_device.context()->VSSetConstantBuffers(0, 1, cbs);
//
//	// Update Vertex and index buffers
//	renderState->m_vertexBuffer = (renderState->m_device.CreateVertexBuffer(m_PolygonDesc.vertices));
//	renderState->m_indexBuffer = (renderState->m_device.CreateIndexBuffer(m_PolygonDesc.indices));
//	ID3D11Buffer* vbs[] = { renderState->m_vertexBuffer.get() };
//
//	//Update strides and offets based on the vertex class
//	UINT strides[] = { sizeof(VertexPositionColor) };
//	UINT offsets[] = { 0 };
//
//	renderState->m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);
//
//	// Watch out for meshes that cannot be covered by ushort
//	renderState->m_device.context()->IASetIndexBuffer(renderState->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
//	renderState->m_device.context()->DrawIndexed(m_PolygonDesc.indices.size(), 0, 0);
//}

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

	ImGui::Text("Samples: ");
	ImGui::SameLine(); ImGui::Text(std::to_string(m_adaptiveRenderingSamples).c_str());

	ImGui::SameLine(); ImGui::Text(m_name.c_str());
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