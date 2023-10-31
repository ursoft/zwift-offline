#pragma once

//Sampo Siltanen 2016
//
//Class to hold DDS BC1(DXT1) compressed filetype data
//See virtual function documentation in Interfaces.h

#include "Interfaces.h"

#include <string>


typedef struct {			/**** Surface pixel format ****/
	unsigned int dwsize;			//Structure size
	unsigned int dwflags;			//Values which indicate what type of data is in the surface
	unsigned int dwfourCC;			//Four-character codes for specifying compressed or custom formats
	unsigned int dwRGBBitCount;		//Number of bits in an RGB (possibly including alpha) format
	unsigned int dwRBitMask;		//Red (or lumiannce or Y) mask for reading color data
	unsigned int dwGBitMask;		//Green (or U) mask for reading color data
	unsigned int dwBBitMask;		//Blue (or V) mask for reading color data
	unsigned int dwABitMask;		//Alpha mask for reading alpha data
} DDS_PIXELFORMAT;

typedef struct {			/**** DDS file header ****/
	unsigned int dwSize;				//Size of structure
	unsigned int dwFlags;				//Flags to indicate which members contain valid data
	unsigned int dwHeight;				//Surface height (in pixels)
	unsigned int dwWidth;				//Surface width (in pixels)
	unsigned int dwPitchOrLinearSize;	//The pitch or number of bytes per scan line in an uncompressed texture
	unsigned int dwDepth;				//Depth of a volume texture (in pixels), otherwise unused
	unsigned int dwMipMapCount;			//Number of mipmap levels, otherwise unused
	unsigned int dwReserved1[11];		//Unused
	DDS_PIXELFORMAT ddspf;				//The pixel format
	unsigned int dwCaps;				//Specifies the complexity of the surfaces stored
	unsigned int dwCaps2;				//Additional detail about the surfaces stored
	unsigned int dwCaps3;				//Unused
	unsigned int dwCaps4;				//Unused
	unsigned int dwReserved2;			//Unused
} DDS_HEADER;


#ifndef MAKEFOURCC
#define MAKEFOURCC(c0,c1,c2,c3) ((unsigned int)(char)(c0)|((unsigned int)(char)(c1)<<8)|((unsigned int)(char)(c2)<<16)|((unsigned int)(char)(c3)<<24))
#endif

#define DDPF_FOURCC 0x4									//Flag to show that texture contains compressed RGB data; dwFourCC contains valid data.
#define DXT1_FOURCC (MAKEFOURCC('D', 'X', 'T', '1'))	//DXT1 value for dwfourCC
#define FLAGS 0x81007									//DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE
#define HEADER_SIZE 124									//Constant in all DDS files
#define INFO_SIZE 32									//Constant in all DDS files
#define DDSCAPS_TEXTURE 0x1000							//Required in DDS header

class DDSFile : public IFiletype {
public:
	DDSFile();
	virtual ~DDSFile();

	void VInitializeFromFile(const std::string& location);
	void VConversionInitialize(uint8_t* uncompressedImageData, unsigned int imageSize, unsigned int width, unsigned int height);
	void VCreateFile(std::basic_ofstream<uint8_t>& outputFile) const;
	unsigned int VGetWidth() const;
	unsigned int VGetHeight() const;
	unsigned int VGetImageByteSize() const;
	uint8_t* VGetUncompressedImageData() const;

private:
	DDS_HEADER* m_pDdsHeader;
	uint8_t* m_mainData;

	//Returns image data in DXT1 compressed format
	uint8_t* DXT1Compress(const uint8_t* const uncompressedData, const unsigned int compressedImageSize, 
							const unsigned int imageSize, const unsigned int width, const unsigned int height) const;
	//Decodes 16bit rbg value to 3 bytes
	uint8_t* decodeRGB(uint16_t color) const;

	//Compresses 3 bytes (rgb) into one 2 bytes value 
	uint16_t compressRGBBytes(const uint8_t * const rgb) const;
};

