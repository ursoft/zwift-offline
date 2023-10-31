#include "DDSFile.h"
#include "MyException.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <math.h>
#include <deque>

DDSFile::DDSFile()
{
	m_pDdsHeader = nullptr;
	m_mainData = nullptr;
}

DDSFile::~DDSFile()
{
	delete[] m_pDdsHeader;
	delete[] m_mainData;
}

void DDSFile::VInitializeFromFile(const std::string & location)
{
	uint8_t* dataBuffer = nullptr;

	m_mainData = nullptr;
	m_pDdsHeader = nullptr;

	std::ifstream file(location, std::ios::binary);
	if (!file)
		throw MyException("Failure to open dds file from location: " + location);

	char dwMagic[4];
	file.read(dwMagic, 4);

	//Check if file is DDS
	if (strncmp(dwMagic, "DDS ", 4) != 0)
		throw MyException("File " + location + " isn't a dds file");

	// Allocate byte memory that will hold the header
	dataBuffer = new uint8_t[sizeof(DDS_HEADER)];
	file.read((char*)dataBuffer, sizeof(DDS_HEADER));

	// Construct the values from the buffers
	m_pDdsHeader = (DDS_HEADER*)dataBuffer;

	//Check valid structure sizes
	if (m_pDdsHeader->dwSize != 124 || m_pDdsHeader->ddspf.dwsize != 32)
		throw MyException("File " + location + " has invalid header(s)");

	//Check is width and height divisible by 4
	if (m_pDdsHeader->dwHeight % 4 != 0 || m_pDdsHeader->dwWidth % 4 != 0)
		throw MyException("File " + location + " does not have width or height divisible by 4");

	//Check if file contains compressed RGB data
	if (m_pDdsHeader->ddspf.dwflags != DDPF_FOURCC)
		throw MyException("File " + location + " texture does not contain compressed RGB data");

	//Check if file is DXT1 compressed
	if (m_pDdsHeader->ddspf.dwfourCC != DXT1_FOURCC)
		throw MyException("File " + location + " is not DXT1 compressed DDS file");

	//Calculate main image size, (blocksize = 8 and packing = 4 for DXT1)
	unsigned int mainImageSize = std::max((unsigned int)1, (m_pDdsHeader->dwWidth + 3) / 4) * std::max((unsigned int)1, (m_pDdsHeader->dwHeight + 3) / 4) * 8;
	if (mainImageSize != m_pDdsHeader->dwPitchOrLinearSize)
		std::cerr << "Warning! Calculated main image size differ from header value" << std::endl;

	//Read the main data
	m_mainData = new uint8_t[mainImageSize];
	file.read((char*)m_mainData, mainImageSize);
}

void DDSFile::VConversionInitialize(uint8_t * uncompressedImageData, unsigned int imageSize, unsigned int width, unsigned int height)
{
	//Create header
	m_pDdsHeader = new DDS_HEADER();
	m_pDdsHeader->dwSize = HEADER_SIZE;
	m_pDdsHeader->dwFlags = FLAGS;
	m_pDdsHeader->dwHeight = height;
	m_pDdsHeader->dwWidth = width;
	m_pDdsHeader->dwPitchOrLinearSize = std::max((unsigned int)1, (m_pDdsHeader->dwWidth + 3) / 4) * std::max((unsigned int)1, (m_pDdsHeader->dwHeight + 3) / 4) * 8;
	m_pDdsHeader->dwDepth = 0;
	m_pDdsHeader->dwMipMapCount = 0;
	m_pDdsHeader->dwReserved1[11];

	//Create pixel format
	DDS_PIXELFORMAT pixelFormat;
	pixelFormat.dwsize = INFO_SIZE;
	pixelFormat.dwflags = DDPF_FOURCC;
	pixelFormat.dwfourCC = DXT1_FOURCC;
	pixelFormat.dwRGBBitCount = 0;
	pixelFormat.dwRBitMask = 0;
	pixelFormat.dwGBitMask = 0;
	pixelFormat.dwBBitMask = 0;
	pixelFormat.dwABitMask = 0;

	m_pDdsHeader->ddspf = pixelFormat;
	m_pDdsHeader->dwCaps = DDSCAPS_TEXTURE;
	m_pDdsHeader->dwCaps2 = 0;
	m_pDdsHeader->dwCaps3 = 0;
	m_pDdsHeader->dwCaps4 = 0;
	m_pDdsHeader->dwReserved2 = 0;

	m_mainData = DXT1Compress(uncompressedImageData, m_pDdsHeader->dwPitchOrLinearSize, imageSize, width, height);
	delete[] uncompressedImageData;
}

void DDSFile::VCreateFile(std::basic_ofstream<uint8_t>& outputFile) const
{
	outputFile.write((uint8_t*)"DDS ", 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->dwSize, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->dwFlags, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->dwHeight, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->dwWidth, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->dwPitchOrLinearSize, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->dwDepth, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->dwMipMapCount, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->dwReserved1, 44);

	outputFile.write((uint8_t*)&m_pDdsHeader->ddspf.dwsize, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->ddspf.dwflags, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->ddspf.dwfourCC, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->ddspf.dwRGBBitCount, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->ddspf.dwRBitMask, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->ddspf.dwGBitMask, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->ddspf.dwBBitMask, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->ddspf.dwABitMask, 4);

	outputFile.write((uint8_t*)&m_pDdsHeader->dwCaps, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->dwCaps2, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->dwDepth, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->dwCaps3, 4);
	outputFile.write((uint8_t*)&m_pDdsHeader->dwReserved2, 4);

	//write pixels
	unsigned int arraySize = m_pDdsHeader->dwPitchOrLinearSize;
	for (unsigned int i = 0; i < arraySize; ++i) {
		outputFile.write((uint8_t*)&m_mainData[i], 1);
	}
	outputFile.close();
}

unsigned int DDSFile::VGetWidth() const
{
	if (m_pDdsHeader == nullptr)
		return 0;
	return m_pDdsHeader->dwWidth;
}

unsigned int DDSFile::VGetHeight() const
{
	if (m_pDdsHeader == nullptr)
		return 0;
	return m_pDdsHeader->dwHeight;
}

unsigned int DDSFile::VGetImageByteSize() const
{
	if (m_pDdsHeader == nullptr)
		return 0;
	return m_pDdsHeader->dwWidth * m_pDdsHeader->dwHeight * 3;
}

uint8_t * DDSFile::VGetUncompressedImageData() const
{
	if (m_mainData == nullptr)
		return nullptr;

	const unsigned int width = m_pDdsHeader->dwWidth;
	const unsigned int height = m_pDdsHeader->dwHeight;

	//Allocate memory for uncompressed image data, width * height * 3 color bytes per pixel
	uint8_t* bytesBuffer = new uint8_t[width * height * 3];

	unsigned int index = 0;
	//uses inverted order in y so that image is represented the correct way, otherwise it would end up upside down
	for (int invertY = height / 4 - 1; invertY >= 0; --invertY) {
		for (unsigned int x = 0; x < width / 4; ++x) {

			//Read block reference colors
			const uint8_t c0_lo = m_mainData[index++];
			const uint8_t c0_hi = m_mainData[index++];
			const uint8_t c1_lo = m_mainData[index++];
			const uint8_t c1_hi = m_mainData[index++];

			//Read block pixel codes
			const uint8_t bits_0 = m_mainData[index++];
			const uint8_t bits_1 = m_mainData[index++];
			const uint8_t bits_2 = m_mainData[index++];
			const uint8_t bits_3 = m_mainData[index++];

			//Form 32 bit instance of block bits
			const uint32_t bits = bits_0 + 256 * (bits_1 + 256 * (bits_2 + 256 * bits_3));

			//Calculate the four colors used
			uint16_t co0 = c0_lo + c0_hi * 256;
			uint16_t co1 = c1_lo + c1_hi * 256;

			//Decode rgb values
			uint8_t* color0Rgb = decodeRGB(co0);
			uint8_t* color1Rgb = decodeRGB(co1);
			uint8_t* color2Rgb = new uint8_t[3];
			uint8_t* color3Rgb = new uint8_t[3];

			//Interpolate mid colors
			if (co0 > co1) {
				color2Rgb[0] = (2 * color0Rgb[0] + color1Rgb[0]) / 3;
				color2Rgb[1] = (2 * color0Rgb[1] + color1Rgb[1]) / 3;
				color2Rgb[2] = (2 * color0Rgb[2] + color1Rgb[2]) / 3;

				color3Rgb[0] = (color0Rgb[0] + 2 * color1Rgb[0]) / 3;
				color3Rgb[1] = (color0Rgb[1] + 2 * color1Rgb[1]) / 3;
				color3Rgb[2] = (color0Rgb[2] + 2 * color1Rgb[2]) / 3;
			}
			else { //Uses third color as alpha value
				color2Rgb[0] = (color0Rgb[0] + color1Rgb[0]) / 2;
				color2Rgb[1] = (color0Rgb[1] + color1Rgb[1]) / 2;
				color2Rgb[2] = (color0Rgb[2] + color1Rgb[2]) / 2;

				color3Rgb[0] = 0;
				color3Rgb[1] = 0;
				color3Rgb[2] = 0;
			}

			for (int invertYi = 3; invertYi >= 0; --invertYi) {
				for (int invertXi = 3, xi = 0; invertXi >= 0; --invertXi, ++xi) {
					int bitPos = 31 - (2 * (invertYi * 4 + invertXi) + 1);
					int code = ((bits >> bitPos) & 3);
					uint8_t rgb[3];

					switch (code) {
					case 0:
						rgb[0] = color0Rgb[0];
						rgb[1] = color0Rgb[1];
						rgb[2] = color0Rgb[2];
						break;
					case 1: 
						rgb[0] = color1Rgb[0];
						rgb[1] = color1Rgb[1];
						rgb[2] = color1Rgb[2];
						break;
					case 2: 
						rgb[0] = color2Rgb[0];
						rgb[1] = color2Rgb[1];
						rgb[2] = color2Rgb[2];
						break;
					case 3:
						rgb[0] = color3Rgb[0];
						rgb[1] = color3Rgb[1];
						rgb[2] = color3Rgb[2];
						break;
					default:
						std::cerr << "Error decoding image, texel had code value " << code << std::endl;
						break;
					}

					//Find position pixel position in buffer so that we dont have to iterate
					unsigned int byteBufferPos = 3 * (width * (4 * invertY + invertYi) + 4 * x + xi);
					bytesBuffer[byteBufferPos] = rgb[0];		//red
					bytesBuffer[byteBufferPos + 1] = rgb[1];	//green
					bytesBuffer[byteBufferPos + 2] = rgb[2];	//blue
				}
			}
			delete[] color0Rgb;
			delete[] color1Rgb;
			delete[] color2Rgb;
			delete[] color3Rgb;
		}
	}

	//linear rgb buffer
	return bytesBuffer;
}

uint8_t * DDSFile::DXT1Compress(const uint8_t* const uncompressedData, const unsigned int compressedImageSize, 
								const unsigned int imageSize, const unsigned int width, const unsigned int height) const
{
	//compressed image data to be returned
	uint8_t* imageData = new uint8_t[compressedImageSize];

	//Values used in calculations
	const unsigned int RGBBytes = 3;
	const unsigned int bytesInCompressedBlock = 8;
	const unsigned int pixelsInBlock = 16;
	const unsigned int pixelsInBlockRow = 4;

	//Loop through all blocks
	int numberOfBlocks = imageSize / (pixelsInBlock * RGBBytes);
	int blockX = 0;
	int blockY = 0;
	for (int blockNum = 0; blockNum < numberOfBlocks; ++blockNum, ++blockX) {
		
		//Keep track of current block
		if (blockNum != 0 && blockNum % (width / pixelsInBlockRow) == 0) {
			blockX = 0;
			++blockY;
		}			

		//Loop through pixels in block to find min and max color values
		uint16_t min16 = 65535; //initialized with max value
		uint16_t max16 = 0;		//initialized with min value
		uint8_t maxRgb[RGBBytes] = { 0, 0, 0 };		//color0
		uint8_t minRgb[RGBBytes] = { 255, 255, 255 };	//color1
		for (int y = 0; y < 4; ++y) {
			for (int x = 0; x < 4; ++x) {

				//Calculate from where to read color bytes
				unsigned int bytePos = RGBBytes * (blockY * pixelsInBlockRow * width + blockX * pixelsInBlockRow + y * width + x);
				uint8_t rgb[RGBBytes] = { uncompressedData[bytePos], uncompressedData[bytePos + 1], uncompressedData[bytePos + 2] };
				uint16_t temp = compressRGBBytes(rgb);

				//save the value if it is the biggest so far
				if (temp > max16) {
					max16 = temp;
					maxRgb[0] = rgb[0];
					maxRgb[1] = rgb[1];
					maxRgb[2] = rgb[2];
				}
				//save the value if it is the smallest so far
				if (temp < min16) {
					min16 = temp;
					minRgb[0] = rgb[0];
					minRgb[1] = rgb[1];
					minRgb[2] = rgb[2];
				}
			}
		}

		//Place blocks in reverse order (invert both x and y)
		unsigned int totalYBlocks = height / pixelsInBlockRow; //Row = column in this case
		unsigned int totalXBlocks = width / pixelsInBlockRow;
		unsigned int compressBytePos = bytesInCompressedBlock * (totalYBlocks * totalXBlocks - blockY * totalXBlocks - totalXBlocks + blockX);

		//form the 4 first bytes from reference colors 
		//								green 3 bits			blue 5 bits
		imageData[compressBytePos++] = ((maxRgb[1] & 28) << 3) | (maxRgb[2] >> 3);		//c0_lo
		//								red 5 bits			green 3 bits
		imageData[compressBytePos++] = (maxRgb[0] & 248) | (maxRgb[1] >> 5);			//c0_hi
		imageData[compressBytePos++] = ((minRgb[1] & 28) << 3) | (minRgb[2] >> 3);		//c1_lo
		imageData[compressBytePos++] = ((minRgb[0] & 248) | minRgb[1] >> 5);			//c1_hi

		uint8_t color2[RGBBytes] = { 0,0,0 };
		uint8_t color3[RGBBytes] = { 0,0,0 };
		if (max16 > min16) {
			color2[0] = (2 * maxRgb[0] + minRgb[0]) / 3;
			color2[1] = (2 * maxRgb[1] + minRgb[1]) / 3;
			color2[2] = (2 * maxRgb[2] + minRgb[2]) / 3;

			color3[0] = (maxRgb[0] + minRgb[0] * 2) / 3;
			color3[1] = (maxRgb[1] + minRgb[1] * 2) / 3;
			color3[2] = (maxRgb[2] + minRgb[2] * 2) / 3;
		}
		else {
			color2[0] = (maxRgb[0] + minRgb[0]) / 2;
			color2[1] = (maxRgb[1] + minRgb[1]) / 2;
			color2[2] = (maxRgb[2] + minRgb[2]) / 2;
		}

		//Loop through pixel colors of block to map colors to reference colors
		for (int y = 3; y >= 0; --y) {
			uint8_t codeByte = 0;
			for (int x = 3; x >= 0; --x) {

				unsigned int bytePos = RGBBytes * (blockY * pixelsInBlockRow * width + blockX * pixelsInBlockRow + y * width + x);
				uint8_t rgb[RGBBytes] = { uncompressedData[bytePos], uncompressedData[bytePos + 1], uncompressedData[bytePos + 2] };
				int code = 0;

				int distanceCo0 = std::abs((maxRgb[2] + 256 * (maxRgb[1] + 256 * maxRgb[0])) - (rgb[2] + 256 * (rgb[1] + 256 * rgb[0])));
				int distanceCo2 = std::abs((color2[2] + 256 * (color2[1] + 256 * color2[0])) - (rgb[2] + 256 * (rgb[1] + 256 * rgb[0])));

				//if color is closer to the second color there is no need to compare color0 again
				if (distanceCo0 > distanceCo2) {
					code = 2;
					int distanceCo3 = std::abs((color3[2] + 256 * (color3[1] + 256 * color3[0])) - (rgb[2] + 256 * (rgb[1] + 256 * rgb[0])));

					if (distanceCo2 > distanceCo3) {
						code = 3;
						int distanceCo1 = std::abs((minRgb[2] + 256 * (minRgb[1] + 256 * minRgb[0])) - (rgb[2] + 256 * (rgb[1] + 256 * rgb[0])));

						if (distanceCo3 > distanceCo1) {
							code = 1;
						}
					}
				}

				//Form byte of 2 bit codes, pixels a to e are in order MSB e -> a LSB
				codeByte = (codeByte | (uint8_t)((code & 3) << 2 * x));
			}
			imageData[compressBytePos++] = codeByte;
		}	
	}
	return imageData;
}

uint8_t* DDSFile::decodeRGB(uint16_t color) const {
	uint8_t* rgb = new uint8_t[3];

	rgb[0] = (color >> 11) & 31;
	rgb[1] = (color >> 5) & 63;
	rgb[2] = (color) & 31;

	rgb[0] = (rgb[0] << 3) | (rgb[0] >> 2);
	rgb[1] = (rgb[1] << 2) | (rgb[1] >> 4);
	rgb[2] = (rgb[2] << 3) | (rgb[2] >> 2);

	return rgb;
}

uint16_t DDSFile::compressRGBBytes(const uint8_t * const rgb) const
{
	uint8_t colorLo = ((rgb[1] & 28) << 3) | (rgb[2] >> 3);																				//								red 5 bits			green 3 bits
	uint8_t colorHi = (rgb[0] & 248) | (rgb[1] >> 5);
	return colorLo + colorHi * 256;
}
