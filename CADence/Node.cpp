#include "Node.h"
#include "imgui.h"

Node* Node::AttachChild(Object* object)
{
	Node* newNode = new Node();
	newNode->m_object = object;
	newNode->m_parent = this;
	m_children.push_back(newNode);
	return newNode;
}

void Node::DetachFromParent()
{
	if (m_parent != nullptr)
	{	
		// delete this node from the parent's children list
		for (int i = 0; i < m_parent->m_children.size(); i++)
		{		
			if (m_parent->m_children[i]->m_object == this->m_object)
			{
				m_parent->m_children.erase(m_parent->m_children.begin() + i);
			}
		}
		// set parent of this object to null
		this->m_parent = nullptr;
	}	
}

void Node::DrawHierarchyNode()
{
	const char* name = m_object->m_name.c_str();

	if (m_children.size() == 0)
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
			for (int i = 0; i < m_children.size(); i++)
			{
				m_children[i]->DrawHierarchyNode();
			}
			ImGui::TreePop();
		}
	}
}

void Node::DrawPopupContextItem(Node* node)
{
	const char* name = node->m_object->m_name.c_str();
	if (ImGui::BeginPopupContextItem(name))
	{
		if (ImGui::Selectable("Rename object"))
		{
			// Trigger a popup for renaming objects - probably check if name is availible through name registry
		}
		if (ImGui::Selectable("Remove object"))
		{		
			Remove();			
		}
		ImGui::EndPopup();
	}
}

// Removes all references to other objects and calls destructor to free memory
void Node::Remove()
{
	// clear this element from parent's children list
	DetachFromParent();
	// Call recursive remove
	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i]->Remove();
	}	
	// Complete the deletion of this object	
	delete m_object;
	delete this; //Fix asap
}

void Node::Render(RenderData* renderData)
{
	if (m_object != nullptr)
		m_object->RenderObject(renderData);
	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i]->Render(renderData);
	}
}

void Node::Update()
{
	if (m_object != nullptr)
		m_object->UpdateObject();
	//TODO [MG] : Calculate tmp transform based on the differrnce between object transform and the given transform

	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i]->Update();
	}
}