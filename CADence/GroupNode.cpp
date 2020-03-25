#include "GroupNode.h"
#include "Scene.h"
#include "imgui.h"

GroupNode::GroupNode(std::vector<std::weak_ptr<Node>> children)
{
	m_children = children;
}

void GroupNode::AddChild(std::weak_ptr<Node> child)
{
	m_children.push_back(child);
}

void GroupNode::DrawNodeGUI(Scene& scene)
{
	std::string hashes = "##";
	std::string labelName = m_object->m_name + hashes + m_object->m_defaultName;	

	if (m_isRenamed)
	{
		DrawRenameGUI();
	}
	else
	{
		bool treePushed = true;
		// Set Leaf Flags
		ImGuiTreeNodeFlags leaf_flags = ImGuiTreeNodeFlags_None;
		if (m_isSelected)
		{
			leaf_flags |= ImGuiTreeNodeFlags_Selected;
		}

		if (m_children.size() == 0)
		{
			treePushed = false;
			leaf_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		// Draw Leaf
		bool open = ImGui::TreeNodeEx(labelName.c_str(), leaf_flags);

		// Process the click of the tree node 
		if (ImGui::IsItemClicked())
		{
			scene.SelectionChanged(*this);
		}
		bool nodeRemoved = false;
		// Create a context menu
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
				nodeRemoved = true;
				m_children.clear();				
				scene.RemoveObject(m_object);
				
			}

			ImGui::EndPopup();
		}

		if (nodeRemoved)
			int i = 2;
		std::string childLabelName;
		// If node is open
		open &= m_children.size() != 0;
		if (open && !nodeRemoved && treePushed)
		{
			bool removed = false;
			auto it = m_children.begin();
			while (it != m_children.end())
			{
				if (auto node = it->lock())
				{
					ImGuiTreeNodeFlags leaf_flags2 = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
					if (node->m_isSelected)
					{
						leaf_flags2 |= ImGuiTreeNodeFlags_Selected;
					}

					childLabelName = node->m_object->m_name + hashes
						+ node->m_object->m_defaultName + hashes + m_object->m_defaultName;
					ImGui::TreeNodeEx(childLabelName.c_str(), leaf_flags2);

					if (ImGui::IsItemClicked())
					{
						scene.SelectionChanged(*node);
					}


					if (ImGui::BeginPopupContextItem(childLabelName.c_str()))
					{
						if (ImGui::Selectable("Remove node"))
						{
							it = m_children.erase(it);
							BezierCurve* c = dynamic_cast<BezierCurve*>(m_object.get());
							c->RemoveChild(node);
							removed = true;
						}
						ImGui::EndPopup();
					}

					if (removed == false) it++;
					else removed = false;
				}
				else {
					it = m_children.erase(it);
				}
			}		
			ImGui::TreePop();
		}	
		if(nodeRemoved && treePushed)
			ImGui::TreePop();
	}
}

void GroupNode::DrawChildGUI()
{

}
