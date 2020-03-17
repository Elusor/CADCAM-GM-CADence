#include "Node.h"
#include "imgui.h"

std::shared_ptr<Node> Node::AttachChild(std::unique_ptr<Object>& object)
{
	std::shared_ptr<Node> newNode = std::shared_ptr<Node>(new Node());
	newNode->m_object = std::move(object);
	newNode->m_parent = this;
	m_children.push_back(newNode);
	return newNode;
}

void Node::DetachFromParent()
{	
	if (m_parent != nullptr)
	{	
		// dte this node from the parent's children list
		for (int i = 0; i < m_parent->m_children.size(); i++)
		{		
			if (m_parent->m_children[i]->m_object == this->m_object)
			{					
				m_parent->m_children.erase(m_parent->m_children.begin() + i);
			}
		}
	}		
}

void Node::DrawHierarchyNode()
{
	const char* name = m_object->m_name.c_str();

	if (m_children.size() == 0)
	{
		ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		// Add unique popup id generator
		DrawPopupContextItem();
	}
	else
	{
		bool nodeOpen = ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick);
		DrawPopupContextItem();
		if (nodeOpen) {
			for (int i = 0; i < m_children.size(); i++)
			{
				m_children[i]->DrawHierarchyNode();
			}
			ImGui::TreePop();
		}
	}
}

void Node::DrawPopupContextItem()
{
	const char* name = m_object->m_name.c_str();
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
	// Call recursive remove
	DetachFromParent();
	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i]->Remove();
	}	

	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i].reset();
	}
	// Complete the deletion of this object	
	m_object.reset();
	

}

void Node::Render(std::unique_ptr<RenderData> & renderData)
{
	if (m_object != nullptr)
		m_object->RenderObject(renderData);
	for (int i = 0; i < m_children.size(); i++)
	{
		m_children[i]->Render(renderData);
	}
}

std::shared_ptr<Node> Node::GetPointerToSelf()
{
	return std::shared_ptr<Node>(this);
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