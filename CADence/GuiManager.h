#pragma once
#include "imgui.h"
#include <string>

class GuiManager {
public:

	void Update();
	/*void EnableDuplicateModal();
	void EnableFileWrongFormatModal();*/
	void EnableCustomModal(std::string message, std::string customHeader);
private:

	std::string m_customHeader;
	std::string m_customMessage;
	bool m_displayDuplicateModal;
	bool m_displayFileWrongFormat;	
	bool m_displatCustomModal;

	void DisplayCustomModal();
	/*void DisplayDuplicateModal();
	void DisplayFileWrongFormatModal();*/
};