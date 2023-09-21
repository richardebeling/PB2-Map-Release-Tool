/*
    This file is part of the Paintball 2 Map Release Tool.

    The Paintball 2 Map Release Tool is free software: you can
	redistribute it and/or modify it under the terms of the
	GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option)
	any later version.

    The Paintball 2 Map Release Tool is distributed in the hope that
	it will be useful, but WITHOUT ANY WARRANTY; without even the
	implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this file.  If not, see <http://www.gnu.org/licenses/>.

	Copyright (C) 2014 Richard Ebeling ("richard")
*/

#include "main.h"
#include "Files.h"
#include "DefaultFiles.h"
#include "resource.h"

//Use ComCtl.dll V6
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HWND g_hWinMain;
HWND g_hStatus;
HWND g_hStatic;
HFONT g_hFont;



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASSEX wincl;
	CHAR szClassName[ ] = "MapReleaseTool\0";
	INITCOMMONCONTROLSEX icex;

	wincl.hInstance = hInstance;
	wincl.lpszClassName = szClassName;
	wincl.lpfnWndProc = WndProc;
	wincl.style = CS_DBLCLKS;
	wincl.cbSize = sizeof (WNDCLASSEX);
	wincl.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_APP));
	wincl.hIconSm = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, 16, 16, LR_SHARED);
	wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	wincl.hbrBackground = (HBRUSH) (COLOR_WINDOW);

	if (!RegisterClassEx (&wincl))
	{
		MessageBox(NULL, "Could not register window class. Will now exit.", NULL, MB_OK | MB_ICONERROR);
		exit(-1);
	}


	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&icex);

	g_hWinMain = CreateWindowEx (WS_EX_ACCEPTFILES,
								szClassName,
								"DP:PB2 Map Release Tool\0",
								WS_OVERLAPPEDWINDOW,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								300,
								150,
								HWND_DESKTOP,
								NULL,
								hInstance,
								NULL);

	//Other windows will be created in "OnCreate".

	if (g_hWinMain == NULL)
	{
		MessageBox(NULL, "Could create main window. Will now exit.", NULL, MB_OK | MB_ICONERROR);
		exit(-1);
	}
	
	ShowWindow(g_hWinMain, nCmdShow);
	UpdateWindow(g_hWinMain);
	
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return (int) msg.wParam;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hWnd, WM_CREATE, OnMainWindowCreate);
		HANDLE_MSG(hWnd, WM_SIZE, OnMainWindowSize);
		HANDLE_MSG(hWnd, WM_DROPFILES, OnMainWindowDropFiles);
		HANDLE_MSG(hWnd, WM_DESTROY, OnMainWindowDestroy);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

static BOOL OnMainWindowCreate (HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	g_hStatus = CreateWindowEx(0,
							   STATUSCLASSNAME,
							   NULL,
							   WS_CHILD | WS_VISIBLE,
							   0, 0, 0, 0,
							   hWnd,
							   NULL,
							   GetModuleHandle(NULL),
							   NULL);

	ChangeStatusText("Waiting for user input...");

	g_hStatic = CreateWindowEx(0,
							   "STATIC",
							   "Drop your .bsp file here",
							   WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
							   0, 0, 0, 0,
							   hWnd,
							   NULL,
							   GetModuleHandle(NULL),
							   NULL);

	HDC hdc = GetDC(NULL);
	LONG lfHeight = -MulDiv(9, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(NULL, hdc);
	g_hFont = CreateFont(lfHeight, 0, 0, 0, 0, FALSE, 0, 0, 0, 0, 0, 0, 0, "MS Shell Dlg\0");

	SendMessage(g_hStatic, WM_SETFONT, WPARAM(g_hFont), true);

	return TRUE;
}

static void OnMainWindowSize (HWND hWnd, UINT state, int cx, int cy)
{
	RECT rc;
	int iStatusHeight;

	SendMessage(g_hStatus, WM_SIZE, 0, 0);

	GetWindowRect(g_hStatus, &rc);
	iStatusHeight = rc.bottom - rc.top;

	GetClientRect(hWnd, &rc);
	MoveWindow(g_hStatic, 0, 0, rc.right, rc.bottom - iStatusHeight, FALSE);
	RedrawWindow(g_hStatic, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}

static void OnMainWindowDropFiles (HWND hWnd, HDROP hDrop)
{
	char*  path;

	if (DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0) != 1)
	{
		MessageBox(g_hWinMain, "This tool can only process one file at the same time.", "Too many files", MB_OK | MB_ICONERROR);
		return;
	}

	UINT size = DragQueryFile(hDrop, 0, NULL, 0) + 1;
	path = new char[size];

	if (!DragQueryFile(hDrop, 0, path, size))
	{
		MessageBox(g_hWinMain, "Error when trying to get the dropped files.", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	DragFinish(hDrop);

	GenerateArchiveFromFile(std::string(path), ChangeStatusText);
}

static void OnMainWindowDestroy (HWND hWnd)
{
	DeleteObject(g_hFont);

	PostQuitMessage(0);
}

static void ChangeStatusText(std::string str)
{
	SendMessage(g_hStatus, SB_SETTEXT, 0, (LPARAM)str.c_str());
}



static void GenerateArchiveFromFile(const std::string &file, void (*ChangeStatusText)(std::string))
{
	BSPFile* bsp;
	std::vector<std::string> requiredFiles;
	MistakesClass mistakes;

	(*ChangeStatusText)("Checking file location...");

	BSPFilePath bspPath(file);
	if (!bspPath.IsOk())
	{
		mistakes.wrongFilePlacing = true;
		mistakes.displayMistakes();
		(*ChangeStatusText)("Waiting for user input...");
		return;
	}

	(*ChangeStatusText)("Reading file...");
	bsp = new BSPFile(bspPath.GetPath());
	if (!bsp->IsOk())
	{
		MessageBox(g_hWinMain, "Error while reading the map file.", "Error", MB_OK | MB_ICONERROR);
		(*ChangeStatusText)("Waiting for user input...");
		return;
	}

	(*ChangeStatusText)("Analyzing compilation...");
	mistakes.noVisingOrLighting = (!bsp->IsVised() || !bsp->IsLighted());

	(*ChangeStatusText)("Analyzing textures...");
	ConcatStringVectorsWithoutDuplicates(&requiredFiles, bsp->GetUsedTextures());

	(*ChangeStatusText)("Analyzing required files in entities...");
	ConcatStringVectorsWithoutDuplicates(&requiredFiles, bsp->RequiredFilesInEntities());
	//requiredfiles in worldspawn; model in func_* / model; sky texture
	delete bsp; //not required anymore

	(*ChangeStatusText)("Analyzing dependencies in required files...");
	AddDependencies(&requiredFiles, bspPath.GetPb2BasePath());
	//adds dependencies in the above, recursively, without a depth limit

	(*ChangeStatusText)("Comparing required files with default files...");
	RemoveDefaultFiles(&requiredFiles);

	(*ChangeStatusText)("Looking for r_script, mapinfo and mapshot...");
	std::string name = bspPath.GetFilename();

	std::string tmp = GetRealFilename(bspPath.GetPb2BasePath(), "pics/mapshots/beta/" + name.substr(0, name.length() - 4) + ".img");
	if (tmp.length() > 0)
		requiredFiles.push_back(tmp);

	tmp = "maps/mapinfo/" + name.substr(0, name.length() - 4) + ".txt";
	if (FileExists(bspPath.GetPb2BasePath() + tmp))
		requiredFiles.push_back(tmp);	

	tmp = "scripts/" + name.substr(0, name.length() - 4) + ".txt";
	if (FileExists(bspPath.GetPb2BasePath() + tmp))
		requiredFiles.push_back(tmp);	

	(*ChangeStatusText)("Generating real filenames...");
	ReplaceWithRealFilenames(&requiredFiles, &mistakes, bspPath.GetPb2BasePath());
	//replace generic extensiosn (.img and .mdl) with the real ones (and adds a .skp file for every .skm)

	(*ChangeStatusText)("Checking textures...");
	CheckTextures(&requiredFiles, &mistakes, bspPath.GetPb2BasePath());
	//power of two? Non-hr4 version linked?
	
	requiredFiles.insert(requiredFiles.end(), bspPath.GetNonBasePath());

	(*ChangeStatusText)("Creating the archive...");
	std::string out = GetOutputPath(bspPath.GetFilename());
	if (out.length() == 0)
	{
		(*ChangeStatusText)("Aborted...");
		return;
	}
	
	if (CreateZip(out, requiredFiles, bspPath.GetPb2BasePath()))
	{
		if (mistakes.isAnythingWrong())
		{
			mistakes.displayMistakes();
		}
		MessageBox(g_hWinMain, "The archive has been created.\n\n"
								"Credits to the developers of the zlib and minizip libraries for providing "
								"a free solution for creating zip files.",
								"Archive successfully created", MB_ICONINFORMATION | MB_OK);
	}
	else
	{
		MessageBox(g_hWinMain, "Fatal error: could not create the archive.", "Fatal Error", MB_ICONERROR | MB_OK);
	}
	
	(*ChangeStatusText)("Waiting for user input...");
}

static void AddDependencies(std::vector<std::string> * requiredFiles, const std::string &pb2path)
{
	for (size_t i = 0; i < requiredFiles->size(); i++)
	{
		//Dependencies from r_scripts
		if (requiredFiles->at(i).length() > 4 && !requiredFiles->at(i).compare(requiredFiles->at(i).length() - 4, 4, ".txt"))
		{
			RScriptFile script((pb2path + requiredFiles->at(i)).c_str());
			if (script.IsOk())
			{
				std::vector<std::string> tmp = script.GetUsedTextures();
				for (size_t x = 0; x < tmp.size(); x++)
				{
					if (std::find(requiredFiles->begin(), requiredFiles->end(), tmp[x]) == requiredFiles->end())
						requiredFiles->insert(requiredFiles->end(), tmp[x]);
				}
			}
		}
		//Dependencies from model files
		else if (requiredFiles->at(i).length() > 4 && !requiredFiles->at(i).compare(requiredFiles->at(i).length() - 4, 4, ".mdl"))
		{
			//If available, use skm model
			std::string path = pb2path + requiredFiles->at(i).substr(0, requiredFiles->at(i).length() - 4) + ".skm";
			SKMFile * skm = new SKMFile(path.c_str());
			if (skm->IsOk())
			{
				std::vector<std::string> tmp = skm->GetUsedTextures(); // will only return texture names, so we need to prepend "models/mymodels/"
				size_t end = requiredFiles->at(i).find_last_of("/");
				for (size_t x = 0; x < tmp.size(); x++)
				{
					tmp[x] = requiredFiles->at(i).substr(0, end + 1) + tmp[x];

					if (std::find(requiredFiles->begin(), requiredFiles->end(), tmp[x]) == requiredFiles->end())
						requiredFiles->insert(requiredFiles->end(), tmp[x]);
				}
			}
			else //use md2 model
			{
				path = pb2path + requiredFiles->at(i).substr(0, requiredFiles->at(i).length() - 4) + ".md2";
				MD2File * md2 = new MD2File(path.c_str());
				if (md2->IsOk())
				{
					std::vector<std::string> tmp = md2->GetUsedTextures();
					for (size_t x = 0; x < tmp.size(); x++)
					{
						if (std::find(requiredFiles->begin(), requiredFiles->end(), tmp[x]) == requiredFiles->end())
							requiredFiles->insert(requiredFiles->end(), tmp[x]);
					}
				}
				delete md2;
			}
			delete skm;
		}
	}
}

static void CheckTextures(std::vector<std::string> * files, MistakesClass * mistakes, const std::string &pb2path)
{
	for (size_t i = 0; i < files->size(); )
	{
		std::string extension = files->at(i).substr(files->at(i).size() - 4);

		if (   extension.compare(".jpg")
			&& extension.compare(".tga")
			&& extension.compare(".wal")
			&& extension.compare(".png")
			&& extension.compare(".pcx")  )
		{
			i++;
			continue; //not a texture file
		}

		if (files->at(i).find("hr4") != std::string::npos)
		{
			mistakes->missingFiles.push_back(files->at(i) + " - always link to the low-res version. File not added.");
			files->erase(files->begin() + i);
			continue; //mapper should link to the low res texture and leave the choice of usign hr4s to the user
		}

		ImageFile img ((pb2path + files->at(i)).c_str());
		if (img.IsOk() && img.IsHeightPowerOfTwo() && img.IsWidthPowerOfTwo())
		{
			size_t pos = files->at(i).find_last_of("/");
			std::string hr4file = files->at(i).substr(0, pos)
								  + "/hr4"
								  + files->at(i).substr(pos, files->at(i).length() - pos - 4)
								  +".img";
			hr4file = GetRealFilename(pb2path, hr4file);

			if (hr4file.length() > 0)
			{
				files->insert(files->begin() + i, hr4file);
				i++; //jump over the created entry to the check for hr4 won't be triggered.
			}
			i++;
		}
		else
		{
			mistakes->missingFiles.push_back(files->at(i) + " - resolution is not power of two. File added.");
			i++;
		}
	}

}

static bool CreateZip(const std::string &path, const std::vector<std::string> &files, const std::string &pb2BasePath)
{
	std::ifstream input;
	std::vector<char> buffer;
	bool ok = true;

	zipFile zf = zipOpen(path.c_str(), APPEND_STATUS_CREATE);

	if (zf == NULL)
        return false;

	for (size_t i = 0; i < files.size(); i++)
	{
		input.open(pb2BasePath + files[i], std::ios::binary | std::ios::ate);
		if (!input.is_open())
		{
			ok = false;
			input.close();
			break;
		}
		std::streamoff size = input.tellg();
		buffer.resize(static_cast<unsigned int>(size));
		input.seekg(0, std::ios::beg);
		input.read((char*)&buffer[0], size);
		input.close();

		zip_fileinfo zfi = { 0 };
		if (S_OK == zipOpenNewFileInZip(zf, files[i].c_str(), &zfi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION))
		{
			if (0 != zipWriteInFileInZip(zf, &buffer[0], static_cast<unsigned int>(buffer.size())))
				ok = false;

            if (0 != zipCloseFileInZip(zf))
                ok = false;
		}
		else
		{
			ok = false;
		}
	}

	if (0 != zipClose(zf, NULL))
	{
		return false;
	}
	else
	{
		return ok;
	}
}

static void ConcatStringVectorsWithoutDuplicates(std::vector<std::string> * vecDest, const std::vector<std::string> &vecSource)
{
	vecDest->insert(vecDest->end(), vecSource.begin(), vecSource.end());

	RemoveMultipleEntries(vecDest);
}

static bool FileExists(const std::string &path)
{
	std::ifstream f(path);
    if (f.good())
	{
        f.close();
        return true;
    }
	else
	{
        f.close();
        return false;
    }   
}

static std::string GetOutputPath(const std::string &bspFilename)
{
	char file[MAX_PATH];
	memset(file, 0, sizeof(file));
	memcpy (file, bspFilename.substr(0, bspFilename.length() - 4).append(".zip").c_str(), bspFilename.length());

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_hWinMain;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = "ZIP file (*.zip)\0*.zip\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = &file[0];
	ofn.nMaxFile = sizeof(file)/sizeof(file[0]);
	ofn.nFileExtension = static_cast<WORD>(bspFilename.length() - 3);
	ofn.lpstrTitle = "Select a location to create the archive";
	ofn.lpstrDefExt = "zip";
	ofn.Flags = OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;

	if (!GetSaveFileName(&ofn))
	{
		return std::string();
	}
	else
	{
		return std::string(ofn.lpstrFile);
	}
}

static std::string GetRealFilename(const std::string &pb2BasePath, const std::string &file)
{
	std::string result;
	std::vector<std::string> validExtensions;

	size_t exStart = file.find_last_of('.');
	if (exStart == std::string::npos)
	{
		return result;
	}
	std::string extension = file.substr(exStart);

	if (!extension.compare(".img"))
	{
		validExtensions.reserve(5);
		validExtensions.push_back(".png");
		validExtensions.push_back(".jpg");
		validExtensions.push_back(".tga");
		validExtensions.push_back(".pcx");
		validExtensions.push_back(".wal");
	}
	else if (!extension.compare(".mdl"))
	{
		validExtensions.reserve(2);
		validExtensions.push_back(".skm");
		validExtensions.push_back(".md2");
	}
	else
	{
		validExtensions.push_back(extension);
	}


	for (size_t i = 0; i < validExtensions.size(); i++)
	{
		if ( FileExists(pb2BasePath + file.substr(0, exStart) + validExtensions.at(i)) )
		{
			result = file.substr(0, exStart) + validExtensions.at(i);
			return result;
		}
	}

	return result;
}

static void RemoveDefaultFiles(std::vector<std::string> * files)
{
	for (size_t x = 0; x < (sizeof(defaultFiles) / sizeof(defaultFiles[0])); x++)
	{
		for (size_t y = 0; y < files->size();) //incrementing will be done manually if nothing is erased
		{
			if (!files->at(y).compare(defaultFiles[x])) //with file ending
				files->erase(files->begin() + y);
			else if (!files->at(y).compare(0, files->at(y).length(), defaultFiles[x], strlen(defaultFiles[x]) - 4)) //without file ending
				files->erase(files->begin() + y);
			else
				y++;
		}
	}

}

static void RemoveMultipleEntries(std::vector<std::string> * vec)
{
	std::sort(vec->begin(), vec->end());
	std::vector<std::string>::iterator last = std::unique(vec->begin(), vec->end());
	vec->erase(last, vec->end());
}

static void ReplaceWithRealFilenames(std::vector<std::string> * requiredFiles, MistakesClass * mistakes, const std::string &pb2BasePath)
{
	for (size_t i = 0; i < requiredFiles->size(); ) //dont increment i every loop in case the element is deleted
	{
		std::string path = GetRealFilename(pb2BasePath, requiredFiles->at(i));
		if (path.length() == 0)
		{
			std::string tmp = requiredFiles->at(i) + " - no such file found. File not added.";
			if (std::find(mistakes->missingFiles.begin(), mistakes->missingFiles.end(), tmp) == mistakes->missingFiles.end())
				mistakes->missingFiles.insert(mistakes->missingFiles.end(), tmp);
			requiredFiles->erase(requiredFiles->begin() + i);
		}
		else	
		{
			requiredFiles->at(i) = path;
			if (!path.substr(path.length() - 4).compare(".skm"))
			{
				requiredFiles->insert(requiredFiles->begin() + i, path = path.substr(0, path.length() - 4).append(".skp"));
				i++;
			}
			i++;
		}
	}
}



void MistakesClass::displayMistakes(void) const
{
	std::string msg = "Some things are not correct. Please fix the following errors:";

	if (wrongFilePlacing)
		msg.append("\n\nErr1: Your fileplacing or naming is wrong. Make sure the bsp file is in pball/maps/beta and has "
					"a '_bX' or '_betaX' extension. The filename should only consist of lower case letters or underscores. "
					"Example: pball/maps/beta/yourmap_beta1.bsp\n"
					"There may be more errors that can't be detected due to missing information about given files likes custom textures.\n");

	if (noVisingOrLighting)
		msg.append("\n\n\nErr2: The mapfile you selected does not have vis-information or lightmap information. "
					"Make sure you perform a final compile with a vising and lighting process.\n");

	if (missingFiles.size() > 0)
	{
		msg.append("\n\nErr3: There are some files who either are not accessable or whose specifications are incorrect. "
				   "Make sure you put your own files in additional folders and all image files are power of 2 resolutions. "
				   "\"File added\" means that the file was added to the archive although it has incorrect specifications\n"
				   "Correct example: pball/textures/yourtextures/yourtexture1.jpg with 128x128.\n\n"
				   "Please check these files (*.img = any kind of image; *.mdl = any kind of model):\n");

		for (size_t i = 0; i < missingFiles.size(); i++)
			msg.append(missingFiles.at(i) + "\n");
	}

	msg.append("\n\nYou may use CTRL+C to copy the content of this message and paste it somewhere else so reading it becomes easier.");

	MessageBox(g_hWinMain, msg.c_str(), "Errors while checking files...", MB_OK | MB_ICONWARNING);
}

bool MistakesClass::isAnythingWrong(void) const
{
	return (wrongFilePlacing || noVisingOrLighting || missingFiles.size() != 0);
}

MistakesClass::MistakesClass()
{
	noVisingOrLighting = false;
	wrongFilePlacing = false;
}