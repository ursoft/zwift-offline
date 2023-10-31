#pragma once

//Sampo Siltanen 2016
//Converter class for BMP-DDS DXT1 converter
//See virtual function documentation in Interfaces.h

#include "Interfaces.h"
#include "BMPFile.h"
#include "DDSFile.h"

#include <map>
#include <memory>
#include <string>

class Converter : public IConverter {
public:
	Converter();
	~Converter();

	void VLoadFile(const std::string& fileLocation);
	void VConvert(const std::string& fromFormat, const std::string& targetFormat);

private:
	//Datastructure that holds key: datatype file ending, value: pointer to valid filetype
	std::map<std::string, std::shared_ptr<IFiletype>> m_filetypes;

	//Loads filetype to datastructure
	void loadFiletype(const std::string& location);
	//Removes filetype from datastructure
	void removeFiletype(const std::string& filetype);
	//returns pointer to valid filetype
	std::shared_ptr<IFiletype> getFiletype(const std::string& filetype) const;
	//creates new filetype, adds it to the datastruture and returns pointer to it
	std::shared_ptr<IFiletype> getUninitializedFiletype(const std::string& filetype);
	//Writes header info
	void writeFile(const std::shared_ptr<IFiletype> pFiletype, const std::string& filetype) const;
	//Checks if given filename already exists
	bool isFilenameVacant(const std::string& filename) const;
};

