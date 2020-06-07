#include "GuiManager.h"

void GuiManager::Update()
{
	if (m_displatCustomModal)
	{
		EnableCustomModal(m_customMessage, m_customHeader);
	}
	DisplayCustomModal();
}

void GuiManager::EnableCustomModal(std::string message, std::string customHeader)
{
	m_customMessage = message;
	m_customHeader = customHeader;
	m_displatCustomModal = true;
	ImGui::OpenPopup(m_customHeader.c_str());
}

void GuiManager::DisplayCustomModal()
{	
	std::string header = m_customHeader;
	if (ImGui::BeginPopupModal(header.c_str()))
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(m_customMessage.c_str());

		if (ImGui::Button("Ok"))
		{
			ImGui::CloseCurrentPopup();
			m_customHeader = "";
			m_customMessage = "";
			m_displatCustomModal = false;
		}
		ImGui::EndPopup();
	}	
}
