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
		// Render object using De Casteljau algorithm
		std::vector<Transform> knots;

		for (int i = 0; i < m_controlPoints.size(); i++)
		{
			if (auto point = m_controlPoints[i].lock())
			{
				knots.push_back(point->m_object->m_transform);
			}
		}

		// Get Bezier Curve Points
		auto points = BezierCalculator::CalculateBezierC0Values(knots, 200);

		std::vector<VertexPositionColor> vertices;
		std::vector<unsigned short> indices;

		for (int i = 0; i < points.size(); i++)
		{
			vertices.push_back(VertexPositionColor{
				points[i].m_pos,
				{1.0f,1.0f,1.0f}
				});
			indices.push_back(i);
		}


		// set render Data ------------------------------

		renderData->m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		//Update content to fill constant buffer
		D3D11_MAPPED_SUBRESOURCE res;
		DirectX::XMMATRIX mvp = m_transform.GetModelMatrix() * renderData->m_camera->GetViewProjectionMatrix();
		//Set constant buffer
		auto hres = renderData->m_device.context()->Map((renderData->m_cbMVP.get()), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		memcpy(res.pData, &mvp, sizeof(DirectX::XMMATRIX));
		renderData->m_device.context()->Unmap(renderData->m_cbMVP.get(), 0);
		ID3D11Buffer* cbs[] = { renderData->m_cbMVP.get() };
		renderData->m_device.context()->VSSetConstantBuffers(0, 1, cbs);

		// Update Vertex and index buffers
		renderData->m_vertexBuffer = (renderData->m_device.CreateVertexBuffer(vertices));
		renderData->m_indexBuffer = (renderData->m_device.CreateIndexBuffer(indices));
		ID3D11Buffer* vbs[] = { renderData->m_vertexBuffer.get() };

		//Update strides and offets based on the vertex class
		UINT strides[] = { sizeof(VertexPositionColor) };
		UINT offsets[] = { 0 };

		renderData->m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);

		// Watch out for meshes that cannot be covered by ushort
		renderData->m_device.context()->IASetIndexBuffer(renderData->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
		renderData->m_device.context()->DrawIndexed(indices.size(), 0, 0);

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
	// Recalculate adaptive rendering??
}
