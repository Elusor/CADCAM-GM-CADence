#include "GroupNode.h"
#include "Scene.h"
#include "imgui.h"

void GroupNode::DrawNodeGUI(Scene& scene)
{
	std::string hashes = "##";
	//std::string labelName = m_object->m_name + hashes + m_object->m_defaultName;
	std::string labelName = "group node"+ hashes + "gn1";

	if (m_isRenamed)
	{
		DrawRenameGUI();
	}
	else
	{
		// Set Leaf Flags
		ImGuiTreeNodeFlags leaf_flags = ImGuiTreeNodeFlags_None;
		if (m_isSelected)
		{
			leaf_flags |= ImGuiTreeNodeFlags_Selected;
		}

		if (m_children.size() == 0)
		{
			leaf_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		// Draw Leaf
		bool open = ImGui::TreeNodeEx(labelName.c_str(), leaf_flags);

		// Process the click of the tree node 
		if (ImGui::IsItemClicked())
		{
			scene.SelectionChanged(*this);
		}

		// Create a context menu
		if (ImGui::BeginPopupContextItem(labelName.c_str()))
		{

			if (ImGui::Selectable("Rename object"))
			{
				m_isRenamed = true;
				// Trigger a popup for renaming objects - probably check if name is availible through name registry
			}

			if (ImGui::Selectable("Remove object"))
			{
				m_children.clear();
				scene.RemoveObject(m_object);
			}

			ImGui::EndPopup();
		}

		// If node is open
		if (open && m_children.size() > 0)
		{
			// Draw each child node
			for (int i = 0; i < m_children.size(); i++)
			{
				ImGuiTreeNodeFlags leaf_flags2 = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				/*if (m_isSelected)
				{
					leaf_flags |= ImGuiTreeNodeFlags_Selected;
				}*/

				// Draw Leaf
				labelName = "gicacaca";
				ImGui::TreeNodeEx(labelName.c_str(), leaf_flags2);

				if (ImGui::BeginPopupContextItem(labelName.c_str()))
				{
					if (ImGui::Selectable("Remove node"))
					{
						m_children.erase(m_children.begin() + i);
					}
					ImGui::EndPopup();
				}
			}
			ImGui::TreePop();
		}		
	}
}

void GroupNode::DrawChildGUI()
{

}
