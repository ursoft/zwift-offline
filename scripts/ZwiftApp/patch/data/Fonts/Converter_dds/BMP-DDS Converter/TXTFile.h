#pragma once
#include "Interfaces.h"
#include <string>
#include <stdint.h>

class TXTFile : public IFiletype {
	unsigned int m_w{}, m_h{}, m_sz{};
	uint8_t *m_pixels{}; //RGB format
public:
	TXTFile() {}
	virtual ~TXTFile() { delete[] m_pixels; }

	void VInitializeFromFile(const std::string& location);
	void VConversionInitialize(uint8_t* uncompressedImageData, unsigned int imageSize, unsigned int width, unsigned int height);
	void VCreateFile(std::basic_ofstream<uint8_t>& outputFile) const;
	unsigned int VGetWidth() const { return m_w; }
	unsigned int VGetHeight() const { return m_h; }
	unsigned int VGetImageByteSize() const { return m_sz; }
	uint8_t* VGetUncompressedImageData() const;
};

