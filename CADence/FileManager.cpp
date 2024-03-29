#include "FileManager.h"
#include "IOExceptions.h"
#include <Shlwapi.h>

std::wstring Pwstr2wstring(PWSTR& pwstring)
{
	std::wstring res(pwstring);
	return res;
}

std::wstring FileManager::OpenFileDialog()
{	
	std::wstring fileName = L"";
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		LPCWSTR xmlTypeName = L"XML file";
		COMDLG_FILTERSPEC filter[] =
		{ { xmlTypeName, L"*.xml"} };
		pFileOpen->SetFileTypes(1, filter);

		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					fileName = Pwstr2wstring(pszFilePath);
					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						if (ValidateFileExtension(pszFilePath) == false)
						{
							throw IncorrectFileExtensionException();
						}
						//MessageBox(NULL, pszFilePath, L"File Path", MB_OK);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	return fileName;
}

std::wstring FileManager::SaveFileDialog()
{	
	std::wstring fileName = L"";
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(hr))
	{
		IFileSaveDialog* pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
			IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileOpen));

		LPCWSTR xmlTypeName = L"XML file";
		COMDLG_FILTERSPEC filter[] =
		{ { xmlTypeName, L"*.xml"} };
		pFileOpen->SetFileTypes(1, filter);
		pFileOpen->SetDefaultExtension(L".xml");
		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);				

					fileName = Pwstr2wstring(pszFilePath);
					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						if (ValidateFileExtension(pszFilePath) == false)
						{
							throw IncorrectFileExtensionException();
						}
						//MessageBox(NULL, pszFilePath, L"File Path", MB_OK);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	return fileName;
}

bool FileManager::ValidateFileExtension(LPCWSTR filename)
{
	return wcscmp(PathFindExtensionW(filename), L".xml") == 0;
}

bool FileManager::FileHasExtension(LPCWSTR filename)
{
	return wcscmp(PathFindExtensionW(filename), L"") == 0;
}
