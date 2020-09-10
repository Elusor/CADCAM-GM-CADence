#pragma once
#include "IntersectionException.h"

struct IntersectionTooManyPointsException : IntersectionException
{
	const char* what() const throw ()
	{
		return "\
Found intersection exceeded the maximum amount of points.\n\
Make sure that loop detection settings are not too high and start again.";
	}
};