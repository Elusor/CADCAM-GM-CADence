#include "Point.h"
#include "vertexStructures.h"
#include "imgui.h"
#include "Scene.h"
#include "Node.h"

using namespace DirectX;
void Point::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	//Update content to fill constant buffer
	XMMATRIX m = m_transform.GetModelMatrix();
	auto Mbuffer = renderState->SetConstantBuffer<XMMATRIX>(renderState->m_cbM.get(), m);
	ID3D11Buffer* cbs[] = { Mbuffer };
	renderState->m_device.context()->VSSetConstantBuffers(1, 1, cbs);

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

	renderState->m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// Update Vertex and index buffers
	renderState->m_vertexBuffer = (renderState->m_device.CreateVertexBuffer(vertices));
	renderState->m_indexBuffer = (renderState->m_device.CreateIndexBuffer(indices));
	ID3D11Buffer* vbs[] = { renderState->m_vertexBuffer.get() };

	//Update strides and offets based on the vertex class
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };

	renderState->m_device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);

	// Watch out for meshes that cannot be covered by ushort
	renderState->m_device.context()->IASetIndexBuffer(renderState->m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	renderState->m_device.context()->DrawIndexed(6, 0, 0);
}

bool Point::CreateParamsGui()
{
	ImGui::Begin("Inspector");
	// Create sliders for torus parameters	
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());
	ImGui::Spacing();	
	bool objectChanged = false;
	float dragSpeed = 0.01f;
	float maxVal = 1000.0f;

	std::string posScale = "##Position" + GetIdentifier();
	ImGui::Text("Position (x,y,z)");
	DirectX::XMFLOAT3 pos = m_transform.GetPosition();
	float posf[3] = { pos.x,pos.y,pos.z };
	objectChanged |= ImGui::DragFloat3(posScale.c_str(), (posf), dragSpeed, -maxVal, maxVal);
	m_transform.SetPosition(posf[0], posf[1], posf[2]);


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
		if (dynamic_cast<BezierCurve*>(currentobject) != nullptr)
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
