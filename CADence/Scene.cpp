#include "Scene.h"
#include "imgui.h"
#include "ObjectFactory.h"
void Scene::RenderScene(std::unique_ptr<RenderData> & renderData)
{
	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i]->Render(renderData);
	}
}

void Scene::UpdateSelectedNode()
{
	if(auto selectedNode = this->m_selectedNode.lock())
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

void Scene::AttachObject(std::unique_ptr<Object> & object)
{
	Node* newNode = new Node();
	newNode->m_object = move(object);
	m_nodes.push_back(std::shared_ptr<Node>(newNode));
}

void Scene::DrawSceneHierarchy()
{
	bool node_open = ImGui::TreeNode("Scene");	
	
	if (ImGui::BeginPopupContextItem("item context menu"))
	{
		if (ImGui::BeginMenu("Add child object"))
		{
			if (ImGui::MenuItem("Torus"))
			{
				std::unique_ptr<Object> obj = ObjectFactory::CreateTorus("Trous");
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

	if (node_open)
	{
		for (int i = 0; i < m_nodes.size(); i++)
		{
			const char* name = m_nodes[i]->m_object->m_name.c_str();

			ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
			// Add unique popup id generator
			if (ImGui::BeginPopupContextItem(name))
			{
				if (ImGui::Selectable("Rename object"))
				{
					// Trigger a popup for renaming objects - probably check if name is availible through name registry
				}

				if (ImGui::Selectable("Remove object"))
				{
					RemoveObject(m_nodes[i]->m_object);
				}

				ImGui::EndPopup();
			}
		}

		ImGui::TreePop();
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
