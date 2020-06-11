#pragma once
#include <exception>

struct IncorrectFileExtensionException : public std::exception
{
	const char* what() const throw ()
	{
		return "Invalid input file extension.";
	}
};