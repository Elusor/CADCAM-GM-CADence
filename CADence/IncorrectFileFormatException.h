#pragma once
#include <exception>

struct IncorrectFileFormatException : public std::exception
{
	const char* what() const throw ()
	{
		return "Input file formatting invalid.";
	}
};