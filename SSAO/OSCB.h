#pragma once

#include <Windows.h>
#include <Commdlg.h>
#include <tchar.h>
#include <vector>
#include <string>

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

namespace OSCB{

	class OpenFileDialog
	{
	public:
		OpenFileDialog(void);

		TCHAR*DefaultExtension;
		TCHAR*FileName;
		TCHAR*Filter;
		int FilterIndex;
		int Flags;
		TCHAR*InitialDir;
		HWND Owner;
		TCHAR*Title;

		bool ShowDialog();
	};

	std::vector<std::string> get_all_files_names_within_folder(std::string folder);

	std::string BrowseFolder(std::string saved_path);

	void MakePath(const std::string& path);

	void MakeCopy(const std::string& srcFileName, const std::string& newFileName);
}