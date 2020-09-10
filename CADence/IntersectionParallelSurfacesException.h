#pragma once
#include "IntersectionException.h"

struct IntersectionParallelSurfacesException : IntersectionException
{
	const char* what() const throw ()
	{
		return "Could not find an intersection.\nIntersection is incorrectly defined.\nAre surfaces parallel?.\n\nTry rearranging the surfaces.";
	}
};