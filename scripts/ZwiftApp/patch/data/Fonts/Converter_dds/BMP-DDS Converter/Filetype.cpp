#include "Interfaces.h"
#include "BMPFile.h"
#include "DDSFile.h"
#include "TXTFile.h"

std::shared_ptr<IFiletype> IFiletype::create(const std::string& type) {
	//Initialize right type of filetype
	if (type == "bmp")
		return std::shared_ptr<IFiletype>(new BMPFile());
	else if (type == "dds")
		return std::shared_ptr<IFiletype>(new DDSFile());
	else if (type == "txt")
		return std::shared_ptr<IFiletype>(new TXTFile());
	else
		return std::shared_ptr<IFiletype>();
}