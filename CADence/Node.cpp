#include "Node.h"
#include "imgui.h"
#include "Scene.h"

void Node::Render(std::unique_ptr<RenderState> & renderState)
{
	if (m_object != nullptr)
	{
		m_object->RenderObject(renderState);
	}	
}

std::vector<std::weak_ptr<Node>> Node::GetSelectedChildren()
{
	return std::vector<std::weak_ptr<Node>>();
}

// Returns children of a given node. By default returns empty std::vector. Can be overloaded.
std::vector<std::weak_ptr<Node>> Node::GetChildren()
{
	return std::vector<std::weak_ptr<Node>>();
}

void Node::ClearChildrenSelection()
{
}

void Node::DrawRenameGUI()
{
	std::string name = "##input";
	int size = 30;
	char* text = new char[size];
	memset(text, 0, sizeof(text));
	m_object->m_name.copy(text, m_object->m_name.size() + 1);
	text[m_object->m_name.size()] = '\0';
	bool entered = ImGui::InputText(name.c_str(), text, sizeof(char) * size, ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::SetItemDefaultFocus();
	ImGui::SetKeyboardFocusHere(-1);
	if (entered)
	{
		m_object->m_name = text;
		m_isRenamed = false;
	}
	delete[] text;
}

void Node::DrawNodeGUI(Scene& scene)
{
	std::string hashes = "##";
	std::string labelName = m_object->m_name + hashes + m_object->m_defaultName;

	if (m_isRenamed)
	{
		DrawRenameGUI();
	}
	else
	{
		ImGuiTreeNodeFlags leaf_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (m_isSelected)
		{
			leaf_flags |= ImGuiTreeNodeFlags_Selected;
		}

		ImGui::TreeNodeEx(labelName.c_str(), leaf_flags);

		// Process the click of the tree node 
		if (ImGui::IsItemClicked())
		{
			scene.SelectionChanged(*this);
		}

		// Add unique popup id generator
		if (ImGui::BeginPopupContextItem(labelName.c_str()))
		{
			m_object->RenderObjectSpecificContextOptions(scene);

			if (ImGui::Selectable("Rename object"))
			{
				m_isRenamed = true;
				// Trigger a popup for renaming objects - probably check if name is availible through name registry
			}

			if (ImGui::Selectable("Remove object"))
			{
				scene.RemoveObject(m_object);
			}

			ImGui::EndPopup();
		}
	}
}

std::string Node::GetLabel()
{
	return m_object->GetLabel();
}

void Node::Update()
{
	if (m_object != nullptr)
	{
		if (m_object->GetIsModified())
		{
			m_object->UpdateObject();
		}
	}
}

void Node::LateUpdate()
{
	this->Update();
}
