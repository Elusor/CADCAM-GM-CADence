#include "Scene.h"
#include "imgui.h"

Node* Node::AttachChild(Object* object)
{
	Node* newNode = new Node();
	newNode->object = object;
	children.push_back(newNode);
	return newNode;
}

void Node::Render(RenderData* renderData)
{
	if (object != nullptr)
		object->RenderObject(renderData);
	for (int i = 0; i < children.size(); i++)
	{
		children[i]->Render(renderData);
	}
}

void Node::DrawHierarchyNode()
{	
	if (children.size() == 0)
	{
		ImGui::TreeNodeEx("asdas123", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);		
		// Add unique popup id generator
		if (ImGui::BeginPopupContextItem("item context menu2"))
		{
			if (ImGui::Selectable("Rename object"))
			{

			}
			if (ImGui::Selectable("Remove object"))
			{

			}
			ImGui::EndPopup();
		}
	}
	else
	{
		bool nodeOpen = ImGui::TreeNode("asdas");
		if (ImGui::BeginPopupContextItem("item context menu1"))
		{
			if (ImGui::Selectable("Rename object"))
			{

			}
			if (ImGui::Selectable("Remove object"))
			{

			}
			ImGui::EndPopup();
		}
		if (nodeOpen) {
			for (int i = 0; i < children.size(); i++)
			{
				children[i]->DrawHierarchyNode();
			}
		}
	}
	ImGui::TreePop();
}

void Node::Update()
{
	if(object != nullptr)
		object->UpdateObject();

	//TODO [MG] : Calculate tmp transform based on the differrnce between object transform and the given transform

	for (int i = 0; i < children.size(); i++)
	{
		children[i]->Update();
	}
}

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
	}	
}