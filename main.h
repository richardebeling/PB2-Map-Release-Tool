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

#ifndef __MAIN_H__
#define __MAIN_H__

#include <vector>
#include <string>

#include <Windows.h>
#include <WindowsX.h>
#include <Commctrl.h>

#include <contrib/minizip/zip.h>

class MistakesClass
{
public:
	MistakesClass();
	bool wrongFilePlacing;
	bool noVisingOrLighting;
	std::vector<std::string> missingFiles;

	bool isAnythingWrong() const;
	void displayMistakes() const;
};

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static BOOL OnMainWindowCreate (HWND hWnd, LPCREATESTRUCT lpCreateStruct);
static void OnMainWindowSize (HWND hWnd, UINT state, int cx, int cy);
static void OnMainWindowDropFiles (HWND hWnd, HDROP hDrop);
static void OnMainWindowDestroy (HWND hWnd);
static void ChangeStatusText (std::string str);

static void GenerateArchiveFromFile(const std::string &file, void (*ChangeStatusText)(std::string));

static void AddDependencies(std::vector<std::string> * files, const std::string &pb2path);
static void CheckTextures(std::vector<std::string> * files, MistakesClass * mistakes, const std::string &pb2path);
static void ConcatStringVectorsWithoutDuplicates(std::vector<std::string> * vecDest, const std::vector<std::string> &vecSource);
static bool CreateZip(const std::string &path, const std::vector<std::string> &files, const std::string &pb2BasePath);
static bool FileExists(const std::string &path);
static std::string GetOutputPath(const std::string &bspFilename);
static std::string GetRealFilename(const std::string &pb2BasePath, const std::string &file);
static void RemoveDefaultFiles(std::vector<std::string> * files);
static void RemoveMultipleEntries(std::vector<std::string> * vec);
static void ReplaceWithRealFilenames(std::vector<std::string> * files, MistakesClass * mistakes, const std::string &pb2path);

#endif