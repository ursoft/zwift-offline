#pragma once

#include <string>
#include <memory>

class IFiletype {
public:
	virtual ~IFiletype() {};

	//Factory method to initialize needed filetype
	//Returns pointer to given type if the type was valid. Otherwise returns nullptr
	static std::shared_ptr<IFiletype> create(const std::string& type);

	//Initializes filetype with data from given file
	//Pre-condition: - Filelocation is valid
	//				 - datatype is valid
	//				 - File has valid headers and valid structure
	//Post-condition: Valid file data is loaded into datastructure
	//Throws MyException if encounters an error
	virtual void VInitializeFromFile(const std::string& location) = 0;

	//Initializes filetype with uncompressed data
	//Pre-condition: - Parameters are valid
	//				 - Filetype is uninitialized
	//Post-condition: Filetype is initialized with valid headers and image data
	//Throws MyException if encounters an error
	virtual void VConversionInitialize(uint8_t* uncompressedImageData, unsigned int imageSize, unsigned int width, unsigned int height) = 0;

	//Creates file with data from datastructure
	//Pre-condition: - Filetype is initialized
	//				 - outputFile is valid filestream
	//Post-condition: - Filetype is valid
	//				  - A valid file based on datastructure is created to the folder where software was launched from
	virtual void VCreateFile(std::basic_ofstream<uint8_t>& outputFile) const = 0;

	//Returns image width
	//Pre-condition: Filetype is initialized
	//Post-condition: Datastructure is in valid state
	virtual unsigned int VGetWidth() const = 0;

	//Returns image height
	//Pre-condition: Filetype is initialized
	//Post-condition: Datastructure is in valid state
	virtual unsigned int VGetHeight() const = 0;

	//Returns size of the image without header(s)
	//Pre-condition: Filetype is initialized
	//Post-condition: Datastructure is in valid state
	virtual unsigned int VGetImageByteSize() const = 0;

	//Returns const pointer to uncompressed image data (24 bits per pixel)
	//Pre-condition: Filetype is initialized
	//Post-condition: Datastructure is valid
	virtual uint8_t* VGetUncompressedImageData() const = 0;
};


class IConverter {
public:
	virtual ~IConverter() {};

	//Loads file into data structure
	//Pre-condition: - Filelocation is valid
	//				 - datatype is valid
	//				 - File has valid headers and valid structure
	//Post-condition: Valid file is loaded into datastructure
	virtual void VLoadFile(const std::string& fileLocation) = 0;

	//Converts file from given format to target format and creates new file as an output
	//Pre-condition: Given filetypes has already been loaded into datastructure
	//Post-condition: Has output only if conversion was valid
	virtual void VConvert(const std::string& fromFormat, const std::string& targetFormat) = 0;
};