
#include "Converter.h"
#include "MyException.h"

#include <functional>
#include <iostream>
#include <fstream>

Converter::Converter()
{
}


Converter::~Converter()
{
}

void Converter::VLoadFile(const std::string & fileLocation)
{
	try {
		loadFiletype(fileLocation);
		std::cout << "File " + fileLocation + " loaded successfully" << std::endl;
	}
	catch (std::exception& e) {
		std::cout << "Error in loadFile(): " << e.what() << std::endl;
	}
}

void Converter::VConvert(const std::string& fromFormat, const std::string& targetFormat)
{
	std::shared_ptr<IFiletype> pFromFiletype;
	std::shared_ptr<IFiletype> pTargetFiletype;
	try {
		//get initilized type for from format
		pFromFiletype = getFiletype(fromFormat);
		//get uninitialized type for target format
		pTargetFiletype = getUninitializedFiletype(targetFormat);

		//initialize target type
		pTargetFiletype->VConversionInitialize(pFromFiletype->VGetUncompressedImageData(), 
												pFromFiletype->VGetImageByteSize(), 
												pFromFiletype->VGetWidth(), 
												pFromFiletype->VGetHeight());
		//Create file of target type
		writeFile(pTargetFiletype, targetFormat);
	}
	catch (std::exception& e) {
		std::cout << "Error in Convert(): " << e.what() << std::endl;
		return;
	}

	//Successful
	std::cout << "Conversion from " + fromFormat + " to " + targetFormat + " was successful" << std::endl;
}

void Converter::loadFiletype(const std::string & location)
{
	//Separate filetype from the filename
	std::size_t dot = location.find('.');
	if (dot == std::string::npos)
		throw MyException("No filetype ending found");
	const std::string filetype = location.substr(dot + 1);
	if (filetype == "")
		throw MyException("No filetype ending found");
	
	//Create filetype
	std::shared_ptr<IFiletype> pFiletype = getUninitializedFiletype(filetype);

	//Initialize filetype with actual file
	try {
		pFiletype->VInitializeFromFile(location);
	}
	catch (MyException) {
		//getUninitializedFiletype inserted uninitilized filetype to datastructure so it needs to be removed
		removeFiletype(filetype);
		throw;
	}
}

void Converter::removeFiletype(const std::string & filetype)
{
	//Remove given filetype from datastructure if there is one, otherwise do nothing
	auto it = m_filetypes.find(filetype);
	if (it != m_filetypes.end()) {
		m_filetypes.erase(it);
	}
}

std::shared_ptr<IFiletype> Converter::getFiletype(const std::string & filetype) const
{
	//Check if given filetype is already loaded
	auto it = m_filetypes.find(filetype);
	if (it == m_filetypes.end()) {
		throw MyException("Filetype " + filetype + " has not been loaded yet");
	}
	return it->second;
}

std::shared_ptr<IFiletype> Converter::getUninitializedFiletype(const std::string & filetype)
{
	//Removes existing same type filetype from datastructure
	removeFiletype(filetype);

	//Initialize filetype
	std::shared_ptr<IFiletype> ptr(IFiletype::create(filetype));
	if (ptr == nullptr)
		throw MyException("Filetype " + filetype + " not supported");

	//insert filetype to datastructure
	m_filetypes.insert(std::make_pair(filetype, ptr));

	//return uninitialized filetype pointer
	return ptr;
}

void Converter::writeFile(const std::shared_ptr<IFiletype> pFiletype, const std::string & filetype) const
{
	std::string defaultFilename = "output";
	int maxCount = 100;
	try {
		std::basic_ofstream<uint8_t> outputFile;

		//Create unique output filename with indexing
		for (int i = 1; i < maxCount; ++i) {
			std::string filename = defaultFilename + std::to_string(i) + "." + filetype;
			//Check if filename is in use
			if (isFilenameVacant(filename)) {
				//open ofstream with vacant filename
				outputFile.open(filename, std::ofstream::out | std::ofstream::binary | std::ofstream::app);
				if (outputFile.is_open()) {
					//Create file
					pFiletype->VCreateFile(outputFile);
					//if this is reached the file creation was successful
					std::cout << "Created file " + filename + " successfully" << std::endl;
					return;
				}
				else
					throw MyException("Could not create output file");
			}
		}
		throw MyException("Ran out of indexes in the name for output file");
	}
	catch (std::ios_base::failure& e) {
		std::string errorM = e.what();
		throw MyException("Error writing file: " + errorM);
	}
}

bool Converter::isFilenameVacant(const std::string & filename) const
{
	//Check if filename is vacant
	try {
		std::ifstream file(filename);
		return file.fail();
	}
	catch (std::ifstream::failure) {
		return false;
	}
	
}
