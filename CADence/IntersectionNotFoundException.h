#pragma once
#include "IntersectionException.h"

struct IntersectionNotFoundException : IntersectionException
{
	const char* what() const throw ()
	{
		return "Could not find an intersection.\nMake Sure the surfaces are intersecting or try intersecting with cursor.";
	}
};