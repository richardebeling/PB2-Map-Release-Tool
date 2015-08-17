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

#ifndef __FILES_H__
#define __FILES_H__

#include <string>
#include <vector>
#include <fstream>		//std::ifstream
#include <stdint.h>		//int types (intX_t)
#include <cstring>		//memcpy
#include <algorithm>	//std::sort, find

//-------------------------------------------------------------------------//
//                                BSP-File                                 //
//-------------------------------------------------------------------------//
/*
struct point3f //12 byte
{
    float x;
    float y;
    float z;
};

struct point3s // 6 byte
{
    int16_t x;
    int16_t y;
    int16_t z;
};
 

struct bsp_lump
{
    uint32_t offset;
    uint32_t length;
};

struct bsp_header //size = 20*8 = 160
{
    uint32_t magic;      // magic number ("IBSP")
    uint32_t version;

    bsp_lump lump[19];   // directory of the lumps
};
//lump[5] = texture information

struct bsp_texinfo //size: 76
{
    point3f u_axis; //12 byte
    float u_offset; //4 byte
   
    point3f v_axis; //12 byte
    float v_offset; //4 byte

    uint32_t flags; //4 byte
    uint32_t value; //4 byte

    char texture_name[32]; //32 byte

    uint32_t next_texinfo; //4 byte
};
*/

class BSPFile
{
public:
	BSPFile(std::string path);
	
	std::vector<std::string> GetUsedTextures(void) const;
	std::vector<std::string> RequiredFilesInEntities(void) const;
	bool IsVised(void) const;
	bool IsLighted(void) const;
	bool IsOk() const;

private:
	static void ReplaceFileEnding(std::string * str);
	std::vector<std::vector<std::pair<std::string, std::string> > > GetEntities(void) const;
	std::vector<char> mBytes;
	bool mIsOk;
};


//-------------------------------------------------------------------------//
//                                MD2-File                                 //
//-------------------------------------------------------------------------//
/*struct md2_header //size = 17*4 = 68
{
	int32_t magic; //"IDP2"
	int32_t version; //8
	int32_t skinwidth;
	int32_t skinheight;
	int32_t framesize;
	int32_t num_skins; //internal offset 20
	int32_t num_xyz;
	int32_t num_st;
	int32_t num_tris;
	int32_t num_glcmds;
	int32_t num_frames;
	int32_t ofs_skins; // internal offset: 44
	int32_t ofs_st;
	int32_t ofs_tris;
	int32_t ofs_frames;
	int32_t ofs_glcmds;
	int32_t ofs_end;
};

struct md2_skin //size 64
{
	unsigned char name [64];
};
*/

class MD2File
{
public:
	MD2File(std::string path);

	std::vector<std::string> GetUsedTextures(void) const;
	bool IsOk() const;

private:
	std::vector<char> mBytes;
	bool mIsOk;
};


//-------------------------------------------------------------------------//
//                                SKM-File                                 //
//-------------------------------------------------------------------------//
/*struct skm_header //size = 4*6=24
{
	char magic[4]; //SKM1
	uint32_t modeltype; //2
	uint32_t filesize;
	uint32_t num_bones;
	uint32_t num_meshes; //offset: 16
	uint32_t ofs_meshes; //offset: 20
};

struct skm_mesh //size = 2*64 + 7*4 = 156
{
	char shadername[64];
	char meshname[64];

	uint32_t num_verts;
	uint32_t num_tris;
	uint32_t num_references;
	uint32_t ofs_verts;	
	uint32_t ofs_texcoords;
	uint32_t ofs_indices;
	uint32_t ofs_references;
};*/


class SKMFile
{
public:
	SKMFile(std::string path);

	std::vector<std::string> GetUsedTextures(void) const;
	bool IsOk() const;

private:
	std::vector<char> mBytes;
	bool mIsOk;
};


//-------------------------------------------------------------------------//
//                             r_script-file                               //
//-------------------------------------------------------------------------//

class RScriptFile
{
public:
	RScriptFile(std::string path);

	std::vector<std::string> GetUsedTextures(void) const;
	bool IsOk() const;

private:
	std::vector<std::string> mWords;
	bool mIsOk;
};


//-------------------------------------------------------------------------//
//                             BSP file path                               //
//-------------------------------------------------------------------------//

class BSPFilePath
{
public:
	BSPFilePath(std::string path);
	std::string GetPb2BasePath() const;
	std::string GetNonBasePath() const;
	std::string GetPath() const;
	std::string GetFilename() const;
	bool IsOk() const;

private:
	std::string mPath;
	std::vector<std::string> mFolders;
	bool mIsOk;
};


//-------------------------------------------------------------------------//
//                               Image-file                                //
//-------------------------------------------------------------------------//

/*struct wal_header //size = 32 + 9*4 + 32 = 100
{
    char name[32];
    uint32_t width;
    uint32_t height;
 
    int32_t ofs[4];
    char next_name[32];
    uint32_t flags;
    uint32_t contents;
    uint32_t value;
};

struct pcx_header //size = 14+48+10+54 = 126
{
	uint8_t  magic; //0x0A
	uint8_t  version;
	uint8_t  encoding;
	uint8_t  bitsPerPixel;
	uint16_t xStart;
	uint16_t yStart;
	uint16_t xEnd;
	uint16_t yEnd;
	uint16_t width; //offset 12
	uint16_t height; //offset 14
	uint8_t  palette[48];
	uint8_t  reserved1; //0x0
	uint8_t  numBitPlanes;
	uint16_t bytesPerLine;
	uint16_t paletteType;
	uint16_t horzScreenSize; 
	uint16_t vertScreenSize;
	uint8_t  reserved2[54]; //0x0
};

struct jpg_header //size 2
{
	uint16_t magic; //0xff, 0xd8
};

struct jpg_sof0_header //size 11
{
	uint16_t identifier; //0xff, 0xc0
	uint16_t length;
	uint8_t precision;
	uint16_t height; //big endian!
	uint16_t width; //big endian!
	uint16_t num_components;
};

struct tga_header //size 16
{
	uint8_t picIdLength;
	uint8_t paletteType;
	uint8_t picType;
	uint16_t ofs_palette;
	uint16_t len_palette;
	uint8_t paletteEntrySize;
	uint16_t zeroPointX;
	uint16_t zeroPointY;
	uint16_t width; //offset 12
	uint16_t height; //offset 14
	uint8_t bitsPerPixel;
	uint8_t attributeType;
};

struct png_header //size: 8
{
	uint64_t magic;
};

struct png_IHDRchunk //always the first chunk, starting at 8
{
	int32_t length;
	int32_t type;

	int32_t width; //offset: 16;
	int32_t height; //offset: 20;

	int32_t color_type;
	int32_t bit_depth;
}
*/

class ImageFile
{
public:
	ImageFile(const char* path);
	int8_t GetType(void) const;
	bool IsOk() const;

	int GetWidth() const;
	int GetHeight() const;
	bool IsWidthPowerOfTwo() const;
	bool IsHeightPowerOfTwo() const;

	static const int8_t TYPE_JPEG = 0;
	static const int8_t TYPE_TGA = 1;
	static const int8_t TYPE_PCX = 2;
	static const int8_t TYPE_WAL = 3;
	static const int8_t TYPE_PNG = 4;

private:
	std::vector<unsigned char> mBytes;
	int8_t mType;
	bool mIsOk;
};

#endif // BSP_H