#pragma once

#include <exception>
#include <string>

struct MyException : public std::exception
{
public:
	MyException(std::string message) : m_message(message) {}
	~MyException() throw () {}
	const char* what() const throw() { return m_message.c_str(); }

private:
	std::string m_message;
};