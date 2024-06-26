#define _CRT_SECURE_NO_WARNINGS
#include "TXTFile.h"
#include "MyException.h"
#include <iostream>
#include <fstream>
#include <cassert>

void TXTFile::VInitializeFromFile(const std::string &location)
{
	//only 2048*2048 supported now
	m_w = m_h = 2048;
	m_sz = 3 * m_w * m_h;
	m_pixels = new uint8_t[m_sz];
	FILE *fin = fopen(location.c_str(), "rt");
	char buf[2051];
	int y = m_h;
	while (fgets(buf, sizeof(buf), fin)) {
		if (y == 0)
			break;
		uint8_t *dest = m_pixels + (m_w * 3) * --y;
		for (int x = 0; x < 2048; x++) {
			switch (buf[x]) {
			case '\xdb': //full block (100%)
				*dest++ = 255;
				*dest++ = 255;
				*dest++ = 255;
				break;
			case '\xb1': //medium shade (50%)
				*dest++ = 0x90;
				*dest++ = 0x90;
				*dest++ = 0x90;
				break;
			case '\xb0': //light shade (25%)
				*dest++ = 0x50;
				*dest++ = 0x50;
				*dest++ = 0x50;
				break;
			default:
				*dest++ = 0;
				*dest++ = 0;
				*dest++ = 0;
				break;
			}
		}
	}
	fclose(fin);
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
