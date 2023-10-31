========================================================================
    CONSOLE APPLICATION : BMP-DDS Converter Project Overview
========================================================================

Sampo Siltanen 
2016

This software works as a image file converter. At the moment the supported filetypes are 
DDS (DXT1/BC1) and BMP (24 bits per pixel).

The architecture of this software is built in a way that new supported image types can be 
added by adding new classes that implement the interface IFiletype. No other parts of the 
software has to be altered to do so.


Commands:
L {<filename> | <file location>}
	- Filename can be used if the file is in the same folder,
	- file location needs '\' to be typed in double like '\\'

C <from filetype> <target filetype>
	- Converts previously loaded <from filetype> and creates a <target filetype> output file
	
Q
	- Closes program


BMP-DDS Converter.vcxproj
BMP-DDS Converter.vcxproj.filters
	Visual studio project files.

BMP-DDS Converter.cpp
    This is the main application source file.

Interfaces.h
	Collection of interfaces in the software.

Filetype.cpp
	Contains definition for factory method in IFiletype (see interfaces.h).

BMPFile.h & .cpp
	BMP filetype that implements IFiletype interface (see interfaces.h)


DDSFile.h & .cpp
	DDS filetype that implements IFiletype interface (see interfaces.h)

Converter.h & .cpp
	Class that implements IConverter interface (see interfaces.h). This class is designed so that 
	it doesn't care what filetypes it is converting. New filetypes can be added with little work.

MyException.h
	Custom exception class to send messages when errors happen