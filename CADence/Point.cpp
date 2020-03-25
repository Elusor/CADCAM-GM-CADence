#include "Point.h"
#include "vertexStructures.h"
#include "imgui.h"
#include "Scene.h"
#include "Node.h"

using namespace DirectX;
void Point::RenderObject(std::unique_ptr<RenderState>& renderData)
{
	//Update content to fill constant buffer
	D3D11_MAPPED_SUBRESOURCE res;
	XMMATRIX mvp = m_transform.GetModelMatrix() * renderData->m_camera->GetViewProjectionMatrix();
	//Set constant buffer
	auto hres = renderData->m_device.context()->Map((renderData->m_cbMVP.get()), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	renderData->m_device.context()->Unmap(renderData->m_cbMVP.get(), 0);
	ID3D11Buffer* cbs[] = { renderData->m_cbMVP.get() };
	renderData->m_device.context()->VSSetConstantBuffers(0, 1, cbs);

	float size = 0.2f;

	std::vector<VertexPositionColor> vertices{
		{{-size,0.0f,0.0f},{1.0f,1.0f,1.0f}},
		{{ size,0.0f,0.0f},{1.0f,1.0f,1.0f}},
		{{0.0f,-size,0.0f},{1.0f,1.0f,1.0f}},
		{{0.0f, size,0.0f},{1.0f,1.0f,1.0f}},
		{{0.0f,0.0f,-size},{1.0f,1.0f,1.0f}},
		{{0.0f,0.0f, size},{1.0f,1.0f,1.0f}}
	};

	std::vector<unsigned short> indices{
	0,1,
	2,3,
	4,5
	};

	renderData->m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

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
	renderData->m_device.context()->DrawIndexed(6, 0, 0);
}

bool Point::CreateParamsGui()
{
	ImGui::Begin("Inspector");
	// Create sliders for torus parameters	
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());
	ImGui::Spacing();
	std::string posX = "Position X##" + m_defaultName;
	std::string posY = "Position Y##" + m_defaultName;
	std::string posZ = "Position Z##" + m_defaultName;

	bool objectChanged = false;
	float dragSpeed = 0.01f;
	float maxVal = 1000.0f;
	objectChanged |= ImGui::DragFloat(posX.c_str(), &(m_transform.m_pos.x), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(posY.c_str(), &(m_transform.m_pos.y), dragSpeed, -maxVal, maxVal);
	objectChanged |= ImGui::DragFloat(posZ.c_str(), &(m_transform.m_pos.z), dragSpeed, -maxVal, maxVal);
	ImGui::End();
	return objectChanged;
}

void Point::RenderObjectSpecificContextOptions(Scene& scene)
{	
	// Get curves that do not containt this point
	std::vector<BezierCurve*> curves;
	for(int i = 0; i < scene.m_nodes.size(); i++)
	{
		auto currentobject = (scene.m_nodes[i]->m_object.get());
		if (typeid(*currentobject) == typeid(BezierCurve))
		{
			BezierCurve* curve = dynamic_cast<BezierCurve*>(currentobject);
			if (curve->IsChild(this->m_parent) == false)
			{
				curves.push_back(curve);
			}			
		}
	}

	// If there are aany - display them in the context menu
	if (curves.size() > 0)
	{
		if (ImGui::BeginMenu("Add to Curve"))
		{
			for (int i = 0; i < curves.size(); i++)
			{

				if (auto parent = (curves[i]->m_parent.lock()))
				{
					auto node = dynamic_cast<GroupNode*>(parent.get());
					if (ImGui::MenuItem(node->GetLabel().c_str()))
					{
						std::weak_ptr<Node> weakParent = this->m_parent;
						if (std::shared_ptr<Node> nodeParent = weakParent.lock())
						{
							node->AddChild(nodeParent);
							curves[i]->AttachChild(nodeParent);
						}
					}
				}				
			}			
			ImGui::EndMenu();
		}
	}

	
}
