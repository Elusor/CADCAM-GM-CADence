#include "Scene.h"
#include "imgui.h"

void Node::AttachChild(Object* object)
{
	Node newNode = Node();
	newNode.object = object;
	children.push_back(newNode);
}

void Node::Update()
{
	object->UpdateObject();
	for (int i = 0; i < children.size(); i++)
	{
		children[i].Update();
	}
}

void Node::Render()
{
	object->RenderObject();
	for (int i = 0; i < children.size(); i++)
	{
		children[i].Render();
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
				children[i].DrawHierarchyNode();
			}
		}
	}
}

void Scene::RenderScene()
{
	rootNode.Render();
}

void Scene::UpdateScene()
{ 
	rootNode.Update();
}

void Scene::AttachObject(Object* object)
{
	rootNode.AttachChild(object);
}

void Scene::DrawSceneHierarchy()
{
	bool node_open = ImGui::TreeNode("Scene");
	if (node_open)
	{
		for (int i = 0; i < rootNode.children.size(); i++)
		{
			rootNode.children[i].DrawHierarchyNode();
		}
	}
	
}
