#pragma once
#include <exception>

struct SceneClassifierNotFoundException : public std::exception
{
	const char* what() const throw ()
	{
		return "No scene object was found in the xml file.";
	}
};