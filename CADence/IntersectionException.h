#pragma once
#include <exception>

struct IntersectionException : public std::exception
{
	const char* what() const throw ()
	{
		return "Something went wrong! Try again with different settings.";
	}
};