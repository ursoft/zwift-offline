#include "BMPFile.h"
#include "MyException.h"

#include <iostream>
#include <fstream>



BMPFile::BMPFile()
{
	m_pBmpHeader = nullptr;
	m_pBmpInfoHeader = nullptr;
	m_pixels = nullptr;
}


BMPFile::~BMPFile()
{
	if (m_pixels != nullptr)
		delete[] m_pixels;
	delete[] m_pBmpHeader;
	delete[] m_pBmpInfoHeader;
}

void BMPFile::VInitializeFromFile(const std::string & location)
{
	uint8_t* dataBuffer[2] = { nullptr, nullptr }; // Header buffers
	m_pixels = nullptr;

	m_pBmpHeader = nullptr;
	m_pBmpInfoHeader = nullptr;

	std::ifstream file(location, std::ios::binary);
	if (!file)
		throw MyException("Failure to open bitmap file from location: " + location);

	// Allocate byte memory that will hold the two headers
	dataBuffer[0] = new uint8_t[sizeof(BITMAPFILEHEADER)];
	dataBuffer[1] = new uint8_t[sizeof(BITMAPINFOHEADER)];
	file.read((char*)dataBuffer[0], sizeof(BITMAPFILEHEADER));
	file.read((char*)dataBuffer[1], sizeof(BITMAPINFOHEADER));

	// Construct the values from the buffers
	m_pBmpHeader = (BITMAPFILEHEADER*)dataBuffer[0];
	m_pBmpInfoHeader = (BITMAPINFOHEADER*)dataBuffer[1];

	// Check if the file is a BMP file
	if (m_pBmpHeader->bfType != BF_TYPE_MB)
		throw MyException("File " + location + " isn't a bitmap file");

	//Check if file is 24 bits per pixel
	if (m_pBmpInfoHeader->biBitCount != BIT_COUNT_24)
		throw MyException("File " + location + " isn't a 24bpp file");

	if (m_pBmpInfoHeader->biWidth * 3 % 4 != 0 || m_pBmpInfoHeader->biHeight * 3 % 4 != 0)
		throw MyException("File " + location + " does not have width or height divisible by 4");

	unsigned int imageSize = m_pBmpInfoHeader->biSizeImage;

	//Check if the header shows the size of the image
	//If the size in header is zero, use calculated value based on offset to data and file size
	if (imageSize == 0) {
		std::cerr << "Warning! File " << location << " has invalid header, doesn't show size of the file" << std::endl;
		imageSize = m_pBmpHeader->bfSize - m_pBmpHeader->bfOffBits;
		m_pBmpInfoHeader->biSizeImage = imageSize;
	}

	// Allocate pixel memory
	m_pixels = new uint8_t[imageSize];

	// Go to where image data starts
	file.seekg(m_pBmpHeader->bfOffBits);

	//read in image data
	//Supports only divisible by 4 width and height so no padding is needed
	file.read((char*)m_pixels, imageSize);

	//BGR format to RGB
	for (unsigned long i = 0; i < imageSize; i += 3) { //3 bytes per pixel
		uint8_t tmpRGB = 0;
		tmpRGB = m_pixels[i];
		m_pixels[i] = m_pixels[i + 2];
		m_pixels[i + 2] = tmpRGB;
	}
}

void BMPFile::VConversionInitialize(uint8_t * uncompressedImageData, unsigned int imageSize, unsigned int width, unsigned int height)
{
	m_pBmpHeader = new BITMAPFILEHEADER();
	m_pBmpInfoHeader = new BITMAPINFOHEADER();

	m_pBmpHeader->bfType = BF_TYPE_MB;
	m_pBmpHeader->bfSize = imageSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	m_pBmpHeader->bfReserved1 = 0;
	m_pBmpHeader->bfReserved2 = 0;
	m_pBmpHeader->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	m_pBmpInfoHeader->biSize = sizeof(BITMAPINFOHEADER);
	m_pBmpInfoHeader->biWidth = width;
	m_pBmpInfoHeader->biHeight = height;
	m_pBmpInfoHeader->biPlanes = NUM_OF_PLANES;
	m_pBmpInfoHeader->biBitCount = BIT_COUNT_24;
	m_pBmpInfoHeader->biCompression = BI_RGB;
	m_pBmpInfoHeader->biSizeImage = imageSize;
	m_pBmpInfoHeader->biXPelsPerMeter = PIXELS_PER_METER; 
	m_pBmpInfoHeader->biYPelsPerMeter = PIXELS_PER_METER;
	m_pBmpInfoHeader->biClrUsed = 0;
	m_pBmpInfoHeader->biClrImportant = 0;

	m_pixels = uncompressedImageData;
}

void BMPFile::VCreateFile(std::basic_ofstream<uint8_t>& outputFile) const
{
	//Write headers with correct length to file
	outputFile.write((uint8_t*)&m_pBmpHeader->bfType, 2);
	outputFile.write((uint8_t*)&m_pBmpHeader->bfSize, 4);
	outputFile.write((uint8_t*)&m_pBmpHeader->bfReserved1, 2);
	outputFile.write((uint8_t*)&m_pBmpHeader->bfReserved2, 2);
	outputFile.write((uint8_t*)&m_pBmpHeader->bfOffBits, 4);

	outputFile.write((uint8_t*)&m_pBmpInfoHeader->biSize, 4);
	outputFile.write((uint8_t*)&m_pBmpInfoHeader->biWidth, 4);
	outputFile.write((uint8_t*)&m_pBmpInfoHeader->biHeight, 4);
	outputFile.write((uint8_t*)&m_pBmpInfoHeader->biPlanes, 2);
	outputFile.write((uint8_t*)&m_pBmpInfoHeader->biBitCount, 2);
	outputFile.write((uint8_t*)&m_pBmpInfoHeader->biCompression, 4);
	outputFile.write((uint8_t*)&m_pBmpInfoHeader->biSizeImage, 4);
	outputFile.write((uint8_t*)&m_pBmpInfoHeader->biXPelsPerMeter, 4);
	outputFile.write((uint8_t*)&m_pBmpInfoHeader->biYPelsPerMeter, 4);
	outputFile.write((uint8_t*)&m_pBmpInfoHeader->biClrUsed, 4);
	outputFile.write((uint8_t*)&m_pBmpInfoHeader->biClrImportant, 4);

	//Copy data to change the order
	unsigned int imageSize = m_pBmpInfoHeader->biSizeImage;
	uint8_t* pCopy = new uint8_t[imageSize];
	memcpy(pCopy, m_pixels, imageSize);

	//RGB format to BGR
	for (unsigned long i = 0; i < imageSize; i += 3) { //3 bytes per pixel
		uint8_t tmpRGB = 0;
		tmpRGB = pCopy[i];
		pCopy[i] = pCopy[i + 2];
		pCopy[i + 2] = tmpRGB;
	}

	//image data
	unsigned int arraySize = m_pBmpInfoHeader->biSizeImage;
	for (unsigned int i = 0; i < arraySize; ++i) {
		outputFile.write((uint8_t*)&pCopy[i], 1);
	}
	outputFile.close();

	delete[] pCopy;
}

unsigned int BMPFile::VGetWidth() const
{
	if (m_pBmpInfoHeader == nullptr)
		return 0;
	return m_pBmpInfoHeader->biWidth;
}

unsigned int BMPFile::VGetHeight() const
{
	if (m_pBmpInfoHeader == nullptr)
		return 0;
	return m_pBmpInfoHeader->biHeight;
}

unsigned int BMPFile::VGetImageByteSize() const
{
	if (m_pBmpInfoHeader == nullptr)
		return 0;
	return m_pBmpInfoHeader->biSizeImage;
}

uint8_t * BMPFile::VGetUncompressedImageData() const
{
	if (m_pixels == nullptr)
		return nullptr;

	//Copy the image data to avoid awkward accidental deleting
	uint8_t* pCopy = new uint8_t[m_pBmpInfoHeader->biSizeImage];
	memcpy(pCopy, m_pixels, m_pBmpInfoHeader->biSizeImage);

	return pCopy;
}
