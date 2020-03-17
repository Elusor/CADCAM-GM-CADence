#include "Node.h"
#include "imgui.h"

void DrawPopupContextItem(Node* node);

Node* Node::AttachChild(Object* object)
{
	Node* newNode = new Node();
	newNode->object = object;
	children.push_back(newNode);
	return newNode;
}

void Node::DrawHierarchyNode()
{
	const char* name = object->m_name.c_str();

	if (children.size() == 0)
	{
		ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		// Add unique popup id generator
		DrawPopupContextItem(this);
	}
	else
	{
		bool nodeOpen = ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick);
		DrawPopupContextItem(this);
		if (nodeOpen) {
			for (int i = 0; i < children.size(); i++)
			{
				children[i]->DrawHierarchyNode();
			}
			ImGui::TreePop();
		}
	}
}

void DrawPopupContextItem(Node* node)
{
	const char* name = node->object->m_name.c_str();
	if (ImGui::BeginPopupContextItem(name))
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

void Node::Render(RenderData* renderData)
{
	if (object != nullptr)
		object->RenderObject(renderData);
	for (int i = 0; i < children.size(); i++)
	{
		children[i]->Render(renderData);
	}
}

void Node::Update()
{
	if (object != nullptr)
		object->UpdateObject();

	//TODO [MG] : Calculate tmp transform based on the differrnce between object transform and the given transform

	for (int i = 0; i < children.size(); i++)
	{
		children[i]->Update();
	}
}