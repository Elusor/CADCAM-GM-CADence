#include "Scene.h"
#include "imgui.h"
#include "ObjectFactory.h"

Scene::Scene()
{
	m_objectFactory = std::unique_ptr<ObjectFactory>(new ObjectFactory());
	m_spawnMarker = m_objectFactory->CreateSpawnMarker();
}

void Scene::AttachObject(std::unique_ptr<Object>& object)
{
	Node* newNode = new Node();
	newNode->m_object = move(object);
	m_nodes.push_back(std::shared_ptr<Node>(newNode));
}

void Scene::RemoveObject(std::unique_ptr<Object>& object)
{	
	for (int i = 0; i < m_nodes.size(); i++)
	{
		if (m_nodes[i]->m_object == object)
		{			
			m_nodes[i]->m_object.reset();
			m_nodes[i].reset();
			m_nodes.erase(m_nodes.begin() + i);
			if (m_selectedNode.expired())
			{
				m_selectedNode.reset();
			}
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

	if (m_selectedNode.expired())
	{
		m_selectedNode.reset();
	}
}

void Scene::DrawScenePopupMenu()
{
	if (ImGui::BeginPopupContextItem("item context menu"))
	{
		if (ImGui::BeginMenu("Add child object"))
		{
			if (ImGui::MenuItem("Torus"))
			{
				std::unique_ptr<Object> obj = m_objectFactory->CreateTorus(m_spawnMarker->m_transform);
				AttachObject(obj);
			}

			if (ImGui::MenuItem("Point"))
			{


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
	const char* name = node->m_object->m_name.c_str();

	ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
	if (ImGui::IsItemClicked())
	{	
		
		m_selectedNode = node;
	}
	// Add unique popup id generator
	if (ImGui::BeginPopupContextItem(name))
	{
		if (ImGui::Selectable("Rename object"))
		{
			// Trigger a popup for renaming objects - probably check if name is availible through name registry
		}

		if (ImGui::Selectable("Remove object"))
		{
			RemoveObject(node->m_object);
		}

		ImGui::EndPopup();
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
				DrawNodePopupMenu(m_nodes[i]);
			}
		}
		else 
		{
			ImGui::TreeNodeEx("<No objects in scene>", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		}		

		ImGui::TreePop();
	}
	
}

void Scene::RenderScene(std::unique_ptr<RenderData>& renderData)
{
	m_spawnMarker->RenderObject(renderData);

	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i]->Render(renderData);
	}
}

void Scene::UpdateScene()
{
	// Currently not in use - if necessary implement setting active objects (i.e. rotating etc.)
	// as dirty and run object->Update only on dirty objects
	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i]->Update();
	}
}

void Scene::UpdateSelectedNode()
{
	if (auto selectedNode = this->m_selectedNode.lock())
	{
		if (selectedNode->m_object != nullptr)
		{
			bool selectedObjectModified = selectedNode->m_object->CreateParamsGui();

			if (selectedObjectModified)
			{
				selectedNode->Update();
			}
		}
	}
}
