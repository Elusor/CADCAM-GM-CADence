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
		ImGui::TreeNodeEx("asdas", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);		
	}
	else
	{
		bool nodeOpen = ImGui::TreeNode("asdas");
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

void Scene::UpdateScene()
{ 
	rootNode.Update();
}

Node* Scene::AttachObject(Object* object)
{
	return (rootNode.AttachChild(object));
}

void Scene::DrawSceneHierarchy()
{
	bool node_open = ImGui::TreeNode("Scene");	
	if (node_open)
	{
		for (int i = 0; i < rootNode.children.size(); i++)
		{
			rootNode.children[i]->DrawHierarchyNode();
		}
	}	
}
