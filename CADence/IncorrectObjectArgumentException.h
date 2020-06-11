#pragma once
#include <exception>

struct IncorrectObjectArgumentException : public std::exception
{
	const char* what() const throw ()
	{
		return "Invalid object creation parameters.";
	}
};