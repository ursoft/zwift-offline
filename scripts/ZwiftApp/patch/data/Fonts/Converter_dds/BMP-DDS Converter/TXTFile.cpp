#include "TXTFile.h"
#include "MyException.h"
#include <iostream>
#include <fstream>
#include <cassert>

void TXTFile::VInitializeFromFile(const std::string & location)
{
	assert(false);//not impl
}

void TXTFile::VConversionInitialize(uint8_t * uncompressedImageData, unsigned int imageSize, unsigned int width, unsigned int height)
{
	m_sz = imageSize; m_w = width; m_h = height;
	m_pixels = uncompressedImageData;
}

void TXTFile::VCreateFile(std::basic_ofstream<uint8_t>& outputFile) const
{
	for (int y = m_h - 1; y >= 0; y--) {
		uint8_t *ptr = m_pixels + (m_w * 3)*y;
		for (auto x = 0u; x < m_w; x++) {
			uint8_t sym = ' ';
			int u24 = *ptr + ptr[1] * 256 + ptr[2] * 65536;
			if (ptr[2] >= 0xF0)
				sym = '\xdb'; //full block (100%)
			else if (ptr[2] >= 0x90)
				sym = '\xb1'; //medium shade (50%)
			else if (ptr[2] >= 0x50)
				sym = '\xb0'; //light shade (25%)
			outputFile.write(&sym, 1);
			ptr += 3;
		}
		outputFile.write((const uint8_t *)"\r\n", 2);
	}
	outputFile.close();
}

uint8_t *TXTFile::VGetUncompressedImageData() const
{
	if (m_pixels == nullptr)
		return nullptr;

	//Copy the image data to avoid awkward accidental deleting
	uint8_t* pCopy = new uint8_t[m_sz];
	memcpy(pCopy, m_pixels, m_sz);

	return pCopy;
}
