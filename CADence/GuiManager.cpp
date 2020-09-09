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
	ID3D11ShaderResourceView* tex1, int width1, int height1, std::string image1Title,
	ID3D11ShaderResourceView* tex2, int width2, int height2, std::string image2Title)
{
	m_customMessage = message;
	m_customHeader = customHeader;
	m_tex1 = tex1;
	m_texHeight1 = height1;
	m_texWidth1 = width1;
	m_image1Title = image1Title;

	m_tex2 = tex2;
	m_texHeight2 = height2;
	m_texWidth2 = width2;
	m_image2Title = image2Title;
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

		auto curPos = ImGui::GetCursorPos();
		int imageSize = 480;
		int imageGap = 8;
		std::string title1 = m_image1Title;
		std::string title2 = m_image2Title;

		auto size1 = ImGui::CalcTextSize(title1.c_str(), title1.c_str() + title1.size()).x;
		auto size2 = ImGui::CalcTextSize(title2.c_str(), title2.c_str() + title2.size()).x;

		float totalText1Offset = (imageSize - size1) / 2.f;
		float totalText2Offset = imageGap + imageSize + (imageSize - size2) / 2.f;

		
		ImGui::SetCursorPos(ImVec2(curPos.x + totalText1Offset, curPos.y));
		ImGui::Text(title1.c_str());

		ImGui::SameLine();

		ImGui::SetCursorPos(ImVec2(curPos.x + totalText2Offset, curPos.y));
		ImGui::Text(title2.c_str());

		if (ImGui::Button("Close"))
		{
			m_customHeader = std::string();
			m_customMessage = std::string();
			m_displayDoubleTextureWindow = false;
		}
		ImGui::End();
	}
}
