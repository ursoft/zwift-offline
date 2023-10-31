//Sampo Siltanen 2016
//
//Software that converts 24 bits per pixel bmp file to DDS BC1(DXT1) compressed file and vice versa
//Implementation done with only standard libraries
//
//BMP-DDS Converter.cpp : Defines the entry point for the console application.

#include "Interfaces.h"
#include "Converter.h"

#include <memory>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>

std::vector<std::string> separateCommandWords(std::string& command);

int main()
{
	try {
		std::cout << "File converter by Sampo Siltanen - 2016 \n"
			<< "Currently supported filetypes: DDS (DXT1/BC1) and BMP (24 bits per pixel) \n \n"
			<< "Commands: \n"
			<< "L {<filepath> | <filename>} \n"
			<< "\t - If filename is used, it has to be in current folder \n"
			<< "\t - If filepath is used, '\\' must be typed in double '\\\\' \n"
			<< "\t - Loads file to datastructure \n \n"
			<< "C <filetype1> <filetype2> \n"
			<< "\t - Creates copy of filetype1 and converts it to filetype2. \n"
			<< "\t \t (filetype1 must be loaded into datastructure beforehand) \n \n"
			<< "Q \n"
			<< "\t - Quits program \n" << std::endl;
		
		std::shared_ptr<IConverter> pConverter(new Converter);
		std::string command = "";
		while (command != "q") {
			std::cout << "Enter command: ";
			std::getline(std::cin, command);
			std::cout << std::endl;
			std::vector<std::string> commands = separateCommandWords(command);
			
			if (commands.size() >= 2) {
				if (commands.at(0) == "l" && commands.size() == 2)
					pConverter->VLoadFile(commands.at(1));
				else if (commands.at(0) == "c" && commands.size() == 3)
					pConverter->VConvert(commands.at(1), commands.at(2));
			}
			std::cout << std::endl;
		}
	}
	catch (...) {
		std::cout << "Something went horribly wrong!" << std::endl;
		return 1;
	}
	return 0;
}

std::vector<std::string> separateCommandWords(std::string& command) {
	std::transform(command.begin(), command.end(), command.begin(), ::tolower);
	std::istringstream iss(command);
	std::vector<std::string> words { std::istream_iterator<std::string>{iss},
									std::istream_iterator<std::string>{} };
	return words;
}