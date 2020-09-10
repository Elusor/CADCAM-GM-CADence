#pragma once
#include "IntersectionException.h"

struct IntersectionTooFewPointsException : IntersectionException
{
	const char* what() const throw ()
	{
		return "\
Found intersection contains too few points.\n\
Make sure that loop detection settings are not too high and start again.\n\
If expected intersection is very short lower minimal intersection point count in settings.\n\
If the problem continues try using the cursor to help with intersecting.";
	}
};