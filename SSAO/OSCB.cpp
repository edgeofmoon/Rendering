#include "OSCB.h"
#include <shlobj.h>
using namespace OSCB;

OpenFileDialog::OpenFileDialog(void)
{
	this->DefaultExtension = 0;
	this->FileName = new TCHAR[MAX_PATH];
	this->Filter = 0;
	this->FilterIndex = 0;
	this->Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	this->InitialDir = 0;
	this->Owner = 0;
	this->Title = 0;
}

bool OpenFileDialog::ShowDialog()
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->Owner;
	ofn.lpstrDefExt = this->DefaultExtension;
	ofn.lpstrFile = this->FileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = this->Filter;
	ofn.nFilterIndex = this->FilterIndex;
	ofn.lpstrInitialDir = this->InitialDir;
	ofn.lpstrTitle = this->Title;
	ofn.Flags = this->Flags;

	GetOpenFileName(&ofn);

	if (_tcslen(this->FileName) == 0) return false;

	return true;
}


std::vector<std::string> OSCB::get_all_files_names_within_folder(std::string folder)
{
	std::vector<std::string> names;
	char search_path[200];
	WIN32_FIND_DATA fd;
	sprintf(search_path, "%s/*.*", folder.c_str());
	WCHAR Wsearch_path[200];
	char DefChar = ' ';
	MultiByteToWideChar(CP_ACP, 0, search_path, -1, Wsearch_path, 200);
	HANDLE hFind = ::FindFirstFile(Wsearch_path, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				char ch[260];
				WideCharToMultiByte(CP_ACP, 0, fd.cFileName, -1, ch, 260, &DefChar, NULL);
				names.push_back(ch);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

// source: http://stackoverflow.com/questions/12034943/win32-select-directory-dialog-from-c-c
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

	if (uMsg == BFFM_INITIALIZED)
	{
		std::string tmp = (const char *)lpData;
		//std::cout << "path: " << tmp << std::endl;
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}

std::string OSCB::BrowseFolder(std::string saved_path)
{
	TCHAR path[MAX_PATH];

	//const char * path_param = saved_path.c_str();

	std::wstring wsaved_path(saved_path.begin(), saved_path.end());
	const wchar_t * path_param = wsaved_path.c_str();

	BROWSEINFO bi = { 0 };
	bi.lpszTitle = L"Browse for folder...";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)path_param;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		//get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);

		//free memory used
		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}

		char DefChar = ' ';
		char ch[260];
		// convert wchar to char
		WideCharToMultiByte(CP_ACP, 0, path,
			-1, ch, 260, &DefChar, NULL);
		return std::string(ch);
	}

	return "";
}

void OSCB::MakePath(const std::string& path){
	std::string command("mkdir ");
	command += path;
	system(command.c_str());
}

void OSCB::MakeCopy(const std::string& srcFileName, const std::string& newFileName){
	std::string command("cp ");
	command += (srcFileName + " " + newFileName);
	system(command.c_str());
}
