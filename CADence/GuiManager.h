#pragma once
#include "imgui.h"
#include <string>
#include <d3d11.h>

class GuiManager {
public:

	void Update();
	/*void EnableDuplicateModal();
	void EnableFileWrongFormatModal();*/
	void EnableCustomModal(std::string message, std::string customHeader);
	void EnableTextureWindow(std::string message, std::string customHeader, ID3D11ShaderResourceView* tex, int width, int height);
private:

	std::string m_customHeader;
	std::string m_customMessage;
	bool m_displayDuplicateModal;
	bool m_displayFileWrongFormat;	
	bool m_displatCustomModal;
	bool m_displayTextureWindow;

	ID3D11ShaderResourceView* m_tex;
	int m_texWidth;
	int m_texHeight;

	void DisplayCustomModal();
	void DisplayTextureWindow();
	/*void DisplayDuplicateModal();
	void DisplayFileWrongFormatModal();*/
};