#pragma once
#include <exception>

struct ArgumentException : public std::exception
{
	const char* what() const throw ()
	{
		return "Argument incorrect!";
	}
};