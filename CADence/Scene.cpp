#include "Scene.h"
#include "imgui.h"
#include "ObjectFactory.h"
#include "GroupNode.h"
#include "generalUtils.h"
Scene::Scene()
{
	m_objectFactory = std::unique_ptr<ObjectFactory>(new ObjectFactory());
	std::unique_ptr<Object> spawnMarker = move(m_objectFactory->CreateSpawnMarker()->m_object);
	m_spawnMarker = std::unique_ptr<SpawnMarker>(dynamic_cast<SpawnMarker *>(spawnMarker.release()));

	std::unique_ptr<Object> grid = move(m_objectFactory->CreateSceneGrid(50)->m_object);
	m_grid = std::unique_ptr<SceneGrid>(dynamic_cast<SceneGrid*>(grid.release()));

	Object* middleMarker = new Object();
	m_middleMarker = std::unique_ptr<Object>(middleMarker);
}

void Scene::AttachObject(std::shared_ptr<Node> node)
{
	m_nodes.push_back(std::shared_ptr<Node>(node));
}

void Scene::RemoveObject(std::unique_ptr<Object>& object)
{	
	for (int i = 0; i < m_nodes.size(); i++)
	{
		if (m_nodes[i]->m_object == object)
		{

			m_nodes[i].reset();
			m_nodes.erase(m_nodes.begin() + i);			
		}
	}
}

void Scene::ClearScene()
{
	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i]->m_object.reset();
		m_nodes[i].reset();
	}

	m_nodes.clear();
	m_selectedNodes.clear();
}

void Scene::DrawScenePopupMenu()
{
	if (ImGui::BeginPopupContextItem("item context menu"))
	{
		if (ImGui::BeginMenu("Add child object"))
		{
			if (ImGui::MenuItem("Torus"))
			{
				AttachObject(m_objectFactory->CreateTorus(m_spawnMarker->GetTransform()));
			}

			if (ImGui::MenuItem("Point"))
			{

				std::shared_ptr<Node> newPoint = m_objectFactory->CreatePoint(m_spawnMarker->GetTransform());
				// Check if any bezier curves are selected, if so - add the point to them

				for (int i = 0; i < m_selectedNodes.size(); i++)
				{
					if (std::shared_ptr<Node> selectedNode = m_selectedNodes[i].lock())
					{						
						if (dynamic_cast<BezierCurve*>(selectedNode->m_object.get()) != nullptr)
						{
							BezierCurve* curves = dynamic_cast<BezierCurve*>(selectedNode->m_object.get());
							auto Node = dynamic_cast<GroupNode*>(selectedNode.get());
							Node->AddChild(newPoint);
							curves->AttachChild(newPoint);
						}
					}
				}

				AttachObject(newPoint);

			}
			if (ImGui::MenuItem("Bezier Curve C0"))
			{
				AttachObject(m_objectFactory->CreateBezierCurve(m_selectedNodes));				
			}

			if (ImGui::BeginMenu("Bezier Curve C2"))
			{
				if (ImGui::MenuItem("B-Spline basis"))
				{
					AttachObject(m_objectFactory->CreateBezierCurveC2(m_selectedNodes, BezierBasis::BSpline));
				}

				if (ImGui::MenuItem("Bernstein basis"))
				{
					AttachObject(m_objectFactory->CreateBezierCurveC2(m_selectedNodes, BezierBasis::Bernstein));
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (ImGui::Selectable("Clear scene"))
		{
			ClearScene();
		}
		ImGui::EndPopup();
	}
}

void Scene::DrawNodePopupMenu(const std::shared_ptr<Node> node)
{	
	
}

void Scene::RenderMiddleMarker(std::unique_ptr<RenderState>& renderState)
{
	if (m_selectedNodes.size() > 0)
	{
		int count = m_selectedNodes.size();
		DirectX::XMVECTOR pos = DirectX::XMVectorZero();

		for (int j = 0; j < m_selectedNodes.size(); j++)
		{
			if (auto node = m_selectedNodes[j].lock())
			{
				DirectX::XMVECTOR posj = DirectX::XMLoadFloat3(&(node->m_object->GetPosition()));
				pos = DirectX::XMVectorAdd(pos, posj);
			}
		}

		float countf = (float)count;

		DirectX::XMFLOAT3 newPos;
		DirectX::XMStoreFloat3(&newPos, pos);

		newPos.x /= countf;
		newPos.y /= countf;
		newPos.z /= countf;

		m_middleMarker->SetPosition(newPos);
		m_middleMarker->RenderCoordinates(renderState);
	}
}

void Scene::DrawSceneHierarchy()
{
	bool node_open = ImGui::TreeNode("Scene");	
	
	DrawScenePopupMenu();

	if (node_open)
	{
		if (m_nodes.size() != 0)
		{
			for (int i = 0; i < m_nodes.size(); i++)
			{
				m_nodes[i]->DrawNodeGUI(*this);				
			}
		}
		else 
		{
			ImGui::TreeNodeEx("<No objects in scene>", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		}		

		ImGui::TreePop();
	}	
}

void Scene::SelectionChanged(Node& node)
{
	// If ctrl is not pressed
	// select only the clicked node

	bool uniqueSelection = ImGui::GetIO().KeyCtrl == false;
	uniqueSelection |= node.m_isVirtual;

	if (uniqueSelection)
	{

		std::weak_ptr<Node> weakNode;
		for (int i = 0; i < m_nodes.size(); i++)
		{		
			// clear selected status
			m_nodes[i]->m_isSelected = false;
			m_nodes[i]->ClearChildrenSelection();

			// Find weak_ptr to node
			if (m_nodes[i]->m_object == node.m_object)
			{
				weakNode = m_nodes[i];
			}

			Object& obj = *(node.m_object.get());			
			auto vNode = FindObjectNode(m_nodes[i]->GetChildren(), obj);
			if (vNode.expired() == false)
			{
				weakNode = vNode;
			}

		}

		m_selectedNodes.clear();

		// select this node
		node.m_isSelected = true;			
		m_selectedNodes.push_back(weakNode);
	}
	else
	{
		// Ctrl is pressed
		// Add not selected node to selection or
		// Remove selected node from selection
		if (node.m_isSelected)
		{
			// deselect this node and
			node.m_isSelected = false;
			// remove this node from m_selectedNodes
			auto it = m_selectedNodes.begin();
			while (it != m_selectedNodes.end())
			{
				if (auto selectedNode = it->lock())
				{
					if (selectedNode->m_object == node.m_object)
					{
						it = m_selectedNodes.erase(it);
						break;
					}
					else
					{
						it++;
					}
				}
			}
		}
		else
		{
			// select this node
			node.m_isSelected = true;
			std::weak_ptr<Node> weakNode;
			Object& obj = *(node.m_object.get());
			auto vNode = FindObjectNode(m_nodes, obj);
			if (vNode)
			{
				weakNode = vNode;
			}

			m_selectedNodes.push_back(weakNode);
		}

	}
}

void Scene::RenderScene(std::unique_ptr<RenderState>& renderState)
{
	m_grid->RenderObject(renderState);
	m_spawnMarker->RenderObject(renderState);	
	for (int i = 0; i < m_nodes.size(); i++)
	{
		// TODO [MG] : check if this item is currently selected		
		m_nodes[i]->Render(renderState);
		auto children = m_nodes[i]->GetChildren();
		for (int j = 0; j < children.size(); j++)
		{
			if (auto child = children[j].lock())
			{
				child->Render(renderState);
			}
		}
	}		

	RenderMiddleMarker(renderState);	
}

void Scene::ClearModifiedTag()
{
	auto it = m_nodes.begin();
	while(it != m_nodes.end())
	{ 
		auto node = it->get();
		auto children = node->GetChildren();
		for (int j = 0; j < children.size(); j++)
		{
			if (auto child = children[j].lock())
			{
				child->m_object->SetModified(false);
			}
		}
		node->m_object->SetModified(false);
		it++;
	}
}

void Scene::UpdateScene()
{
	UpdateSelectedNode();

	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i]->Update();
	}
}

void Scene::LateUpdate()
{
	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i]->LateUpdate();
	}
}

void Scene::UpdateSelectedNode()
{	
	// Draw the inspector window
	// foreach selected node
	//   Draw section for each node

	auto it = m_selectedNodes.begin();
	while (it != m_selectedNodes.end())
	{
		// TODO [MG] : add collapsing headers
		if (auto selectedNode = it->lock())
		{
			if (selectedNode->m_object)
			{
				if (selectedNode->m_object->CreateParamsGui())
				{
					selectedNode->m_object->SetModified(true);
				}
			}
			it++;
		}
		else {
			it = m_selectedNodes.erase(it);
		}
	}
}
