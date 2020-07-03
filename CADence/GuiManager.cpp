#include "GuiManager.h"

void GuiManager::Update()
{
	if (m_displatCustomModal)
	{
		EnableCustomModal(m_customMessage, m_customHeader);
	}
	DisplayCustomModal();
	DisplayTextureWindow();
	DisplayDoubleTextureWindow();
}

void GuiManager::EnableCustomModal(std::string message, std::string customHeader)
{
	m_customMessage = message;
	m_customHeader = customHeader;
	m_displatCustomModal = true;
	ImGui::OpenPopup(m_customHeader.c_str());
}

void GuiManager::EnableTextureWindow(std::string message, std::string customHeader, ID3D11ShaderResourceView* tex, int width, int height)
{
	m_customMessage = message;
	m_customHeader = customHeader;
	m_tex1 = tex;
	m_texHeight1 = height;
	m_texWidth1 = width;
	m_displayTextureWindow = true;
}

void GuiManager::EnableDoubleTextureWindow(
	std::string message, std::string customHeader, 
	ID3D11ShaderResourceView* tex1, int width1, int height1, 
	ID3D11ShaderResourceView* tex2, int width2, int height2)
{
	m_customMessage = message;
	m_customHeader = customHeader;
	m_tex1 = tex1;
	m_texHeight1 = height1;
	m_texWidth1 = width1;

	m_tex2 = tex2;
	m_texHeight2 = height2;
	m_texWidth2 = width2;
	m_displayDoubleTextureWindow = true;
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
			m_customHeader = std::string();
			m_customMessage = std::string();
			m_displatCustomModal = false;
		}
		ImGui::EndPopup();
	}	
}

void GuiManager::DisplayTextureWindow()
{
	if (m_displayTextureWindow)
	{
		ImGui::Begin(m_customHeader.c_str());
		ImGui::Text(m_customMessage.c_str());
		ImGui::Image((void*)m_tex1, ImVec2(m_texWidth1, m_texHeight1));
		if (ImGui::Button("Close"))
		{
			m_customHeader = std::string();
			m_customMessage = std::string();
			m_displayTextureWindow = false;
		}
		ImGui::End();
	}
}

void GuiManager::DisplayDoubleTextureWindow()
{
	if (m_displayDoubleTextureWindow)
	{
		ImGui::Begin(m_customHeader.c_str());
		ImGui::Text(m_customMessage.c_str());
		ImGui::Image((void*)m_tex1, ImVec2(m_texWidth1, m_texHeight1));
		ImGui::SameLine();
		ImGui::Image((void*)m_tex2, ImVec2(m_texWidth2, m_texHeight2));
		if (ImGui::Button("Close"))
		{
			m_customHeader = std::string();
			m_customMessage = std::string();
			m_displayDoubleTextureWindow = false;
		}
		ImGui::End();
	}
}
