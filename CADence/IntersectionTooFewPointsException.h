#pragma once
#include "IntersectionException.h"

struct IntersectionTooFewPointsException : IntersectionException
{
	const char* what() const throw ()
	{
		return 
			"Found intersection contains too few points.\n\\
			Make sure that loop detection settings are not too high and start again.\n\\
			If the problem continues try using the cursor to help with intersecting.";
	}
};