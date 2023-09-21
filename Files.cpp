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

	Copyright (C) 2023 Richard Ebeling
*/

#include "Files.h"

//-------------------------------------------------------------------------//
//                                BSP-File                                 //
//-------------------------------------------------------------------------//

BSPFile::BSPFile(std::string path)
{
	std::ifstream stream(path, std::ios::binary | std::ios::ate);
	std::streamoff size;
	
	if (!stream)
	{
		mIsOk = false;
		return;
	}
	
	size = stream.tellg();

	if (size < 160) //bsp file header size
	{
		mIsOk = false;
		return;
	}

	mBytes.resize(static_cast<unsigned int>(size));
	
	stream.seekg(0, std::ios::beg);
	stream.read((char*)&mBytes[0], size);
	
	uint32_t version = 0x26;

	if (memcmp(&mBytes[0], "IBSP", 4) || memcmp(&mBytes[4], &version, 4))
		mIsOk = false;
	else
		mIsOk = true;
}

bool BSPFile::IsOk(void) const
{
	return mIsOk;
}

std::vector<std::string> BSPFile::GetUsedTextures(void) const
{
	std::vector<std::string> result;

	uint32_t start = *((uint32_t*)(&mBytes[4+4 + 5*8])); //location for texture information section start
	uint32_t length = *((uint32_t*)(&mBytes[4+4 + 5*8 + 4])); //length of texture information section
	
	for (uint32_t pos = start; pos < (start + length); pos += 76) //1 texinfo struct = 80 bytes
	{
		char name[33] = { 0 };
		memcpy(name, &mBytes[static_cast<size_t>(pos) + 12+4+12+4 + 4+4], 32);

		if (std::find(result.begin(), result.end(), name) == result.end())
		{
			result.push_back(name);
		}
	}
	
	for (auto& texture : result)
	{
		texture = "textures/" + texture;
		size_t exStart;

		if ((exStart = texture.find('.', texture.length() - 4)) != std::string::npos)
			texture = texture.substr(0, texture.length() - exStart) + ".img";
		else
			texture = texture + ".img";
	}

	return result;
}

std::vector<std::vector<std::pair<std::string, std::string> > > BSPFile::GetEntities(void) const
{
	std::vector<std::vector<std::pair<std::string, std::string> > > result;
	std::string entities;
	std::string currentEntity;

	uint32_t start = *((uint32_t*)(&mBytes[4+4])); //location for entity section start
	uint32_t length = *((uint32_t*)(&mBytes[4+4 + 4])); //entity section length

	entities.assign(&mBytes[start], length);

	size_t entitiesPos = 0;
	size_t entitiesOldpos = 1;

	result.reserve(std::count(entities.begin(), entities.end(), '{'));

	while ((entitiesPos = entities.find('}', entitiesOldpos)) != std::string::npos)
	{
		std::vector<std::pair<std::string, std::string> > values;

		currentEntity.assign(entities.substr(entitiesOldpos, entitiesPos - entitiesOldpos));
		entitiesOldpos = entities.find('{', entitiesPos);

		values.reserve(std::count(currentEntity.begin(), currentEntity.end(), '\n'));

		size_t currentPos = 0;
		size_t currentOldpos = 0;
		while ((currentPos = currentEntity.find('\n', currentOldpos)) != std::string::npos)
		{
			std::string line = currentEntity.substr(currentOldpos, currentPos - currentOldpos);
			currentOldpos = currentPos + 1;

			size_t start = line.find('"') + 1;
			size_t end = line.find('"', start);

			if (end == std::string::npos)
				continue;

			std::string variable = line.substr(start, end - start);

			start = line.find('"', end + 1) + 1;
			end = line.find('"', start);

			if (end == std::string::npos)
				continue;

			std::string value = line.substr(start, end - start);
			if (value.length() > 0 && !value.substr(value.length() - 1).compare("\r"))
				value = value.substr(0, value.length() - 1);

			values.push_back(std::pair<std::string, std::string>(variable, value));
		}

		result.push_back(values);
	}

	return result;
}

std::vector<std::string> BSPFile::RequiredFilesInEntities(void) const
{
	std::vector<std::string> result;
	std::vector<std::vector<std::pair<std::string, std::string> > > entities = GetEntities();

	for (const auto& entity : entities)
	{
		for (const auto& [key, value] : entity)
		{
			if (key == "requiredfiles") //eg. classname worldspawn: "requiredfiles" "scripts/flame1.txt pics/mapshots/mapshot.jpg"
			{
				size_t pos = 0;
				size_t oldpos = 0;

				result.reserve(result.size() + std::count(value.begin(), value.end(), ' '));

				while ((pos = value.find(' ', oldpos)) != std::string::npos)
				{
					std::string currentFile = value.substr(oldpos, pos - oldpos);
					ReplaceFileEnding(&currentFile);

					result.push_back(currentFile);
					oldpos = pos + 1;
				}
				std::string currentFile = value.substr(oldpos);
				ReplaceFileEnding(&currentFile);

				result.push_back(value.substr(oldpos));
			}
			else if (key == "model") //eg. classname func_model: "model" "models/props/keyboard/keyboard1.md2"
			{
				if (value.compare(0, 1, "*"))	//there's an internal reference possible with *X, so this does not relate to an actual model file
				{
					std::string currentFile = value;
					ReplaceFileEnding(&currentFile);
					result.push_back(currentFile);
				}
			}
			else if (key == "sky")
			{
				result.reserve(result.size() + 6);
				std::string path = value.substr(0, value.find(" "));
				result.push_back("env/" + path + "up.img");
				result.push_back("env/" + path + "dn.img");
				result.push_back("env/" + path + "lf.img");
				result.push_back("env/" + path + "ft.img");
				result.push_back("env/" + path + "rt.img");
				result.push_back("env/" + path + "bk.img");
			}
			else if (key == "noise") // e.g. classname target_speaker: "noise" "author/mozart"
			{
				std::string filename = "sound/" + value;
				// add .wav file ending if it wasn't already there (future-proof: also check for all other 3 or 4 digit endings (.ogg, .mp3, .flac, ...)
				if(filename.at(filename.length() - 4) != '.' && filename.at(filename.length() - 5) != '.')
				{
					filename = filename + ".wav";
				}
				result.push_back(filename);
			}
		}
	}

	return result;
}

void BSPFile::ReplaceFileEnding(std::string * str)
{
	std::string filename = str->substr(0, str->length() - 4);
	std::string extension = str->substr(str->length() - 4);

	if (extension == ".md2" || extension == ".skm")
	{
		str->assign(filename + ".mdl");
	}

	else if (extension == ".jpg"
		  || extension == ".tga"
		  || extension == ".pcx"
		  || extension == ".wal")
	{
		str->assign(filename + ".img");
	}
}

bool BSPFile::IsVised(void) const
{
	uint32_t length = (uint32_t)(*(&mBytes[4+4 + 3*8 + 4]));
	return length > 0;
}

bool BSPFile::IsLighted(void) const
{
	uint32_t length = (uint32_t)(*(&mBytes[4+4 + 7*8 + 4]));
	return length > 0;
}


//-------------------------------------------------------------------------//
//                                MD2-File                                 //
//-------------------------------------------------------------------------//

MD2File::MD2File(std::string path)
{
	std::ifstream stream(path, std::ios::binary | std::ios::ate);
	std::streamoff size;
	
	if (!stream)
	{
		mIsOk = false;
		return;
	}
	
	size = stream.tellg();

	if (size < 68) //size md2 files header
	{
		mIsOk = false;
		return;
	}

	mBytes.resize(static_cast<unsigned int>(size));
	
	stream.seekg(0, std::ios::beg);
	stream.read((char*)&mBytes[0], size);
	
	int32_t version = 8;

	if (memcmp(&mBytes[0], "IDP2", 4) || memcmp(&mBytes[4], &version, 4))
		mIsOk = false;
	else
		mIsOk = true;
}

std::vector<std::string> MD2File::GetUsedTextures (void) const
{
	std::vector<std::string> result;

	uint32_t ofs_skins = *((uint32_t*)(&mBytes[44])); //offset ofs_skins
	uint32_t num_skins = *((uint32_t*)(&mBytes[20])); //offset num_skins

	for (uint32_t i = ofs_skins; i < (ofs_skins + (num_skins * 64)); i += 64) //size md2_skin
	{
		char name[65] = { 0 };
		memcpy(name, &mBytes[i], 64);
		std::string str(name);
		size_t exStart;

		if ((exStart = str.find('.', str.length() - 4)) != std::string::npos)
			str = str.substr(0, exStart) + ".img";
		else
			str = str + ".img";

		if (std::find(result.begin(), result.end(), str) == result.end())
			result.push_back(str);
	}

	return result;
}

bool MD2File::IsOk(void) const
{
	return mIsOk;
}


//-------------------------------------------------------------------------//
//                                SKM-File                                 //
//-------------------------------------------------------------------------//

SKMFile::SKMFile(std::string path)
{
	std::ifstream stream(path, std::ios::binary | std::ios::ate);
	std::streamoff pos;
	
	if (!stream)
	{
		mIsOk = false;
		return;
	}
	
	pos = stream.tellg();

	if (pos < 24) //skm header size
	{
		mIsOk = false;
		return;
	}

	mBytes.resize(static_cast<unsigned int>(pos));
	
	stream.seekg(0, std::ios::beg);
	stream.read((char*)&mBytes[0], pos);
	
	int32_t type = 2;

	if (memcmp(&mBytes[0], "SKM1", 4) || memcmp(&mBytes[4], &type, 4))
		mIsOk = false;
	else
		mIsOk = true;
}

std::vector<std::string> SKMFile::GetUsedTextures(void) const
{
	std::vector<std::string> result;

	uint32_t ofs_meshes = (uint32_t)(*(&mBytes[20])); //offset ofs_meshes
	uint32_t num_meshes = (uint32_t)(*(&mBytes[16])); //offset num_meshes

	for (uint32_t i = ofs_meshes; i < (ofs_meshes + (num_meshes * 156)); i += 156) //size skm_mesh
	{
		char shadername[65] = { 0 };
		memcpy(shadername, &mBytes[i], 64);
		std::string str(shadername);
		size_t exStart;

		if ((exStart = str.find('.', str.length() - 4)) != std::string::npos)
			str = str.substr(0, exStart) + ".img";
		else
			str = str + ".img";

		result.push_back(str);
	}

	return result;
}

bool SKMFile::IsOk(void) const
{
	return mIsOk;
}


//-------------------------------------------------------------------------//
//                              RScript-File                               //
//-------------------------------------------------------------------------//

RScriptFile::RScriptFile(std::string path)
{
	std::ifstream stream(path, std::ios::binary | std::ios::ate);
	std::string line;
	
	if (!stream)
	{
		mIsOk = false;
		return;
	}

	stream.seekg(0, std::ios::beg);
	
	while (std::getline(stream, line)) //TODO: This doesn't care about comments in the file. But then again the question is:
	{								   //Should users upload r_scripts including comments? I think it's ok if it doesn't care.
		size_t linePos = 0;
		size_t lineOldpos = 0;

		if (line.length() == 0)
			continue;

		if (line.substr(line.length() - 1, 1) == "\r")
			line = line.substr(0, line.length() - 1);

		while ((linePos = line.find_first_of("\t ", lineOldpos)) != std::string::npos) //space or tab
		{
			if ((linePos - lineOldpos) > 0)
				mWords.push_back(line.substr(lineOldpos, linePos - lineOldpos));

			lineOldpos = linePos + 1;
		}
		if ((line.length() - lineOldpos) > 1)
			mWords.push_back(line.substr(lineOldpos));
	}

	mIsOk = true;
}

std::vector<std::string> RScriptFile::GetUsedTextures(void) const
{
	std::string found;
	std::vector<std::string> result;

	for(size_t i = 0; i < (mWords.size() - 1); i++) //prevent oor error when accessing mWords[i+1]
	{
		if (mWords[i] == "map")
		{
			size_t exStart;

			if ((exStart = mWords[i+1].find('.', mWords[i+1].length() - 4)) != std::string::npos)
				found.assign(mWords[i+1].substr(0, exStart) + ".img");
			else
				found.assign(mWords[i+1] + ".img");

			if(std::find(result.begin(), result.end(), found) == result.end())
				result.push_back(found);
		}
		else if (mWords[i] == "anim")
		{
			size_t texCount = std::find(mWords.begin() + i, mWords.end(), "end") - mWords.begin() + i;
			result.reserve(result.size() + texCount);

			for (i = i+2; (mWords[i].compare("end") != 0) && (i < mWords.size()); i++)
			{
				size_t exStart;
				if ((exStart = mWords[i].find('.', mWords[i].length() - 4)) != std::string::npos)
					found.assign(mWords[i].substr(0, exStart) + ".img");
				else
					found.assign(mWords[i] + ".img");

				if(std::find(result.begin(), result.end(), found) == result.end())
					result.push_back(found);
			}
		}
	}
	return result;
}

bool RScriptFile::IsOk(void) const
{
	return mIsOk;
}

//-------------------------------------------------------------------------//
//                             BSP file path                               //
//-------------------------------------------------------------------------//

BSPFilePath::BSPFilePath(std::string path)
{
	std::string sFilename;

	mIsOk = true;

	size_t pos = 0;
	size_t oldpos = 0;

	mFolders.reserve(std::count(path.begin(), path.end(), '/') + std::count(path.begin(), path.end(), '\\') + 1);

	while(pos < path.length())
	{
		pos = path.find_first_of("\\/", oldpos);
		mFolders.push_back(path.substr(oldpos, pos - oldpos));
		oldpos = pos + 1;
	}

	if (mFolders.at(mFolders.size() - 4) != "pball"
		|| mFolders.at(mFolders.size() - 3) != "maps"
		|| mFolders.at(mFolders.size() - 2) != "beta")
	{
		mIsOk = false;
		return;
	}

	sFilename = mFolders.at(mFolders.size() - 1);
	if (sFilename.find_first_not_of("abcdefghijklmnopqrstuvxwyz1234567890_") != sFilename.length() - 4) //".bsp"
	{
		mIsOk = false;
		return;
	}

	if (sFilename.substr(sFilename.length() - 4) != ".bsp")
	{
		mIsOk = false;
		return;
	}

	pos = sFilename.find("_b");
	if (pos == std::string::npos)
	{
		pos = sFilename.find("_beta");
		if (pos == std::string::npos)
		{
			mIsOk = false;
			return;
		}
		pos += 5;
	}
	else
	{
		pos += 2;
	}

	if (sFilename.find_first_not_of("0123456789", pos) != sFilename.length() - 4) //".bsp"
	{
		mIsOk = false;
	}
}

//Returns "C:/Games/Paintball2/pball/"
std::string BSPFilePath::GetPb2BasePath(void) const
{
	std::string result;

	for (size_t pos = 0; pos < (mFolders.size() - 3); pos++)
	{
		result.append(mFolders[pos]);
		result.append("/");
	}

	return result;
}

//Returns "maps/beta/mymap_b1.bsp"
std::string BSPFilePath::GetNonBasePath(void) const
{
	std::string result;

	for (size_t pos = (mFolders.size() - 3); pos < mFolders.size(); pos++)
	{
		result.append(mFolders[pos]);
		if (pos != (mFolders.size() - 1))result.append("/");
	}

	return result;
}

std::string BSPFilePath::GetFilename(void) const
{
	return mFolders.at(mFolders.size() - 1);
}

//Returns "C:/Games/Paintball2/pball/maps/beta/test_b1.bsp"
std::string BSPFilePath::GetPath(void) const
{
	std::string result;

	for (size_t pos = 0; pos < mFolders.size(); pos++)
	{
		result.append(mFolders[pos]);
		if (pos != (mFolders.size() - 1))
			result.append("/");
	}

	return result;
}

bool BSPFilePath::IsOk(void) const
{
	return mIsOk;
}


//-------------------------------------------------------------------------//
//                              Image file                                 //
//-------------------------------------------------------------------------//

ImageFile::ImageFile(const char* pPath)
{
	std::string path(pPath);
	std::string extension = path.substr(path.length() - 4);
	std::ifstream stream(path, std::ios::binary | std::ios::ate);
	std::streamoff size;

	mType = TYPE_JPEG;
	mIsOk = false;

	if (!stream)
		return;
	
	size = stream.tellg();

	mBytes.resize(static_cast<unsigned int>(size));
	
	stream.seekg(0, std::ios::beg);
	stream.read((char*)&mBytes[0], size);

	if (extension == ".wal" && size >= 100) //size wal_header
	{
		char name[33] = { 0 };
		memcpy(name, &mBytes[0], 32); //name offset

		if (path.find(name) != std::string::npos)
		{
			mType = TYPE_WAL;
			mIsOk = true;
		}
	}
	else if (extension == ".jpg" && size >= 2) //size jpg_header
	{
		uint16_t magic = *((uint16_t*)((&mBytes[0]))); //magic offset
		if(magic == 0xd8ff)
		{
			mType = TYPE_JPEG;
			mIsOk = true;
		}
	}
	else if (extension == ".pcx" && size >= 126) //size pcs_header
	{
		uint8_t magic = *((uint8_t*)((&mBytes[0]))); //magic offset
		if (magic == 0x0A)
		{
			mType = TYPE_PCX;
			mIsOk = true;
		}
	}
	else if (extension == ".tga" && size >= 16) //size tga_header
	{
		uint8_t paletteType = *((uint8_t*)((&mBytes[1]))); //palette type offset
		if (paletteType >= 0 && paletteType <= 1)
		{
			mType = TYPE_TGA;
			mIsOk = true;
		}
	}
	else if (extension == ".png" && size >= 8) //size png_header
	{
		uint64_t magic = *((uint64_t*)((&mBytes[0]))); //magic offset
		if (magic == 0x0A1A0A0D474E5089)
		{
			mType = TYPE_PNG;
			mIsOk = true;
		}
	}
}

uint32_t ImageFile::GetHeight(void) const
{
	switch (mType)
	{
		case TYPE_WAL:
		{
			uint32_t height = (uint32_t)(*(&mBytes[32 + 4])); //height offset
			return height;
		}

		case TYPE_PCX:
		{
			uint16_t height = *((uint16_t*)(&mBytes[14]));
			return height;
		}

		case TYPE_TGA:
		{
			uint16_t height = *((uint16_t*)(&mBytes[14]));
			return height;
		}

		case TYPE_PNG:
		{
			int32_t height = *((int32_t*)(&mBytes[16]));

			int32_t reversedHeight = 0;
			reversedHeight += (height & 0xFF000000) >> 24;
			reversedHeight += (height & 0x00FF0000) >> 8;
			reversedHeight += (height & 0x0000FF00) << 8;
			reversedHeight += (height & 0x000000FF) << 24;

			return reversedHeight;
		}

		case TYPE_JPEG:
		{
			size_t offset = 0;
			for (offset = 0; offset < (mBytes.size() - 1); offset++)
			{
				if ( *((uint16_t*)&mBytes[offset]) == 0xc0ff)
					break;
			}

			if (mBytes.size() < (offset + 7))
				return -1;

			uint16_t height = *((uint16_t*)(&mBytes[offset + 5])); //change endianess!
			uint16_t reversedHeight = 0;
			reversedHeight += (height & 0x00FF) << 8;
			reversedHeight += (height & 0xFF00) >> 8;

			return reversedHeight;
		}
	}
	return -1;
}

uint32_t ImageFile::GetWidth(void) const
{
	switch (mType)
	{
		case TYPE_WAL:
		{
			uint32_t width = (uint32_t)(*(&mBytes[32])); //width offset
			return width;
		}

		case TYPE_PCX:
		{
			uint16_t width = *((uint16_t*)(&mBytes[12]));
			return width;
		}

		case TYPE_TGA:
		{
			uint16_t width = *((uint16_t*)(&mBytes[12]));
			return width;
		}

		case TYPE_PNG:
		{
			int32_t width = *((int32_t*)(&mBytes[20]));

			int32_t reversedWidth = 0;
			reversedWidth += (width & 0xFF000000) >> 24;
			reversedWidth += (width & 0x00FF0000) >> 8;
			reversedWidth += (width & 0x0000FF00) << 8;
			reversedWidth += (width & 0x000000FF) << 24;

			return reversedWidth;
		}

		case TYPE_JPEG:
		{
			size_t offset = 0;
			for (offset = 0; offset < (mBytes.size() - 1); offset++)
			{
				if ( *((uint16_t*)&mBytes[offset]) == 0xc0ff)
					break;
			}

			if (mBytes.size() < (offset + 9))
				return -1;

			uint16_t width = *((uint16_t*)(&mBytes[offset + 7])); //change endianess!
			uint16_t reversedWidth = 0;
			reversedWidth += (width & 0x00FF) << 8;
			reversedWidth += (width & 0xFF00) >> 8;

			return reversedWidth;
		}
	}
	return -1;
}

bool ImageFile::IsHeightPowerOfTwo(void) const
{
	return std::has_single_bit(GetHeight());
}

bool ImageFile::IsWidthPowerOfTwo(void) const
{
	return std::has_single_bit(GetWidth());
}

bool ImageFile::IsOk(void) const
{
	return mIsOk;
}

int8_t ImageFile::GetType(void) const
{
	return mType;
}