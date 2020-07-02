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
	void EnableDoubleTextureWindow(
		std::string message, std::string customHeader,
		ID3D11ShaderResourceView* tex1, int width1, int height1,
		ID3D11ShaderResourceView* tex2, int width2, int height2);
private:

	std::string m_customHeader;
	std::string m_customMessage;

	bool m_displatCustomModal;
	bool m_displayTextureWindow;
	bool m_displayDoubleTextureWindow;
	ID3D11ShaderResourceView* m_tex1;
	int m_texWidth1;
	int m_texHeight1;

	ID3D11ShaderResourceView* m_tex2;
	int m_texWidth2;
	int m_texHeight2;

	void DisplayCustomModal();
	void DisplayTextureWindow();
	void DisplayDoubleTextureWindow();
	/*void DisplayDuplicateModal();
	void DisplayFileWrongFormatModal();*/
};