#pragma once

#include <iostream>
#include <zlib.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <sstream>

typedef uint32_t DWORD;

struct DDS_PIXELFORMAT {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
};

typedef struct {
	DWORD           dwSize;
	DWORD           dwFlags;
	DWORD           dwHeight;
	DWORD           dwWidth;
	DWORD           dwPitchOrLinearSize;
	DWORD           dwDepth;
	DWORD           dwMipMapCount;
	DWORD           dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD           dwCaps;
	DWORD           dwCaps2;
	DWORD           dwCaps3;
	DWORD           dwCaps4;
	DWORD           dwReserved2;
} DDS_HEADER;

#pragma pack(push)
#pragma pack(1)
typedef struct {
	uint16_t        wDummy0;
	uint16_t        wDummy2;
	DWORD           dwDummy0;
	DWORD           dwDummy0a;
	uint16_t        wWidth;
	uint16_t        wHeight;
	uint16_t        wType;
} TGAX_HEADER;
#pragma pack(pop)