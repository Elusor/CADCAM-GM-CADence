#include "Point.h"
#include "vertexStructures.h"
#include "imgui.h"
#include "Scene.h"
#include "Node.h"

using namespace DirectX;

Point::Point(float size)
{
	m_size = size;

	m_meshDesc.m_selectedColor = XMFLOAT3(1.0f, 1.0f, 0.0f);
	m_meshDesc.m_adjustableColor = XMFLOAT3(1.0f, 0.0f, 1.0f);
	m_meshDesc.m_defaultColor = m_meshDesc.m_adjustableColor;

	std::vector<VertexPositionColor> vertices{
		{{-m_size,0.0f,0.0f}, m_meshDesc.m_defaultColor},
		{{ m_size,0.0f,0.0f}, m_meshDesc.m_defaultColor},
		{{0.0f,-m_size,0.0f}, m_meshDesc.m_defaultColor},
		{{0.0f, m_size,0.0f}, m_meshDesc.m_defaultColor},
		{{0.0f,0.0f,-m_size}, m_meshDesc.m_defaultColor},
		{{0.0f,0.0f, m_size}, m_meshDesc.m_defaultColor}
	};

	std::vector<unsigned short> indices{
	0,1,
	2,3,
	4,5
	};

	
	m_meshDesc.vertices = vertices;
	m_meshDesc.indices = indices;
	m_meshDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

}

void Point::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	RenderMesh(renderState, m_meshDesc);
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
			if (curve->IsChild(this->m_nodePtr) == false)
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

				if (auto parent = (curves[i]->m_nodePtr.lock()))
				{
					auto node = dynamic_cast<GroupNode*>(parent.get());
					if (ImGui::MenuItem(node->GetLabel().c_str()))
					{
						std::weak_ptr<Node> weakParent = this->m_nodePtr;
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

void Point::UpdateObject()
{	
	std::vector<VertexPositionColor> vertices{
		{{-m_size,0.0f,0.0f}, m_meshDesc.m_defaultColor},
		{{ m_size,0.0f,0.0f}, m_meshDesc.m_defaultColor},
		{{0.0f,-m_size,0.0f}, m_meshDesc.m_defaultColor},
		{{0.0f, m_size,0.0f}, m_meshDesc.m_defaultColor},
		{{0.0f,0.0f,-m_size}, m_meshDesc.m_defaultColor},
		{{0.0f,0.0f, m_size}, m_meshDesc.m_defaultColor}
	};

	std::vector<unsigned short> indices{
	0,1,
	2,3,
	4,5
	};

	m_meshDesc.vertices = vertices;
	m_meshDesc.indices = indices;
}

void Point::Scale(DirectX::XMFLOAT3 scale)
{
}

void Point::Rotate(DirectX::XMFLOAT3 scale)
{
}
