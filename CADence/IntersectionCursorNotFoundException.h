#pragma once
#include <exception>

struct IntersectionCursorNotFoundException : public std::exception
{
	const char* what() const throw ()
	{
		return "Could not find an intersection.\nMake Sure the surfaces are intersecting or move the cursor to a different position.";
	}
};