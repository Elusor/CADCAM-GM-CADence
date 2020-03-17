#include "Scene.h"
#include "imgui.h"

void Scene::RenderScene(RenderData* renderData)
{
	rootNode.Render(renderData);
}

void Scene::UpdateSelectedNode()
{
	auto selectedNode = this->m_selectedNode;
	if (selectedNode != nullptr)
	{
		bool selectedObjectModified = selectedNode->object->CreateParamsGui();

		if (selectedObjectModified)
		{
			//TODO [MG]: Recalculate selected node and all the children
			selectedNode->Update();
		}
		selectedNode->Update();
	}
}

void Scene::UpdateScene()
{ 
	// Currently not in use - if necessary implement setting active objects (i.e. rotating etc.)
	// as dirty and run object->Update only on dirty objects
	rootNode.Update();
}

Node* Scene::AttachObject(Object* object)
{
	return (rootNode.AttachChild(object));
}

void Scene::DrawSceneHierarchy()
{
	bool node_open = ImGui::TreeNode("Scene");	
	
	if (ImGui::BeginPopupContextItem("item context menu"))
	{
		if (ImGui::BeginMenu("Add child object"))
		{
			ImGui::MenuItem("Torus");
			ImGui::MenuItem("Point");
			ImGui::EndMenu();
		}

		if (ImGui::Selectable("Clear scene"))
		{ 
		}
		ImGui::EndPopup();
	}

	if (node_open)
	{
		for (int i = 0; i < rootNode.children.size(); i++)
		{
			rootNode.children[i]->DrawHierarchyNode();
		}
		ImGui::TreePop();
	}	
}