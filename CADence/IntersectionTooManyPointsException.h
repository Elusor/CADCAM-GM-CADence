#pragma once
#include "IntersectionException.h"

struct IntersectionTooManyPointsException : IntersectionException
{
	const char* what() const throw ()
	{
		return "\
Found intersection exceeded the maximum amount of points.\n\
An intersection will still be added to the scene altough it will have only the points found so far.\n\
If you wish, you may disable this functionality in intersection settings.\n\
Make sure that loop detection settings are not too high and start again.";
	}
};