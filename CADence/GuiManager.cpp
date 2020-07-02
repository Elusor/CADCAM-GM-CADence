#include "GuiManager.h"

void GuiManager::Update()
{
	if (m_displatCustomModal)
	{
		EnableCustomModal(m_customMessage, m_customHeader);
	}
	DisplayCustomModal();
	DisplayTextureWindow();
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
	/*m_customMessage = message;
	m_customHeader = customHeader;*/
	m_tex = tex;
	m_texHeight = height;
	m_texWidth = width;
	m_displayTextureWindow = true;
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

void GuiManager::DisplayTextureWindow()
{
	if (m_displayTextureWindow)
	{
		ImGui::Begin("DirectX11 Texture Test");
		ImGui::Text("pointer = %p", m_tex);
		ImGui::Text("size = %d x %d", m_texWidth, m_texHeight);
		ImGui::Image((void*)m_tex, ImVec2(m_texWidth, m_texHeight));
		if (ImGui::Button("Ok"))
		{
			m_customHeader = "";
			m_customMessage = "";
			m_displayTextureWindow = false;
		}
		ImGui::End();
	}
}