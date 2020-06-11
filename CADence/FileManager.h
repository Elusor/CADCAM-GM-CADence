#pragma once
#include <windows.h>
#include <shobjidl.h> 
#include <string>

class FileManager
{
public:
	std::wstring OpenFileDialog();
	std::wstring SaveFileDialog();
private:
	bool ValidateFileExtension(LPCWSTR filename);
};
