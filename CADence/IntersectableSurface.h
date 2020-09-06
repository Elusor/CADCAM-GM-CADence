#pragma once
#include "IntersectionData.h"
#include "Trimmer.h"

class IntersectableSurface
{
public:
	void SetIntersectionData(IntersectionData data);
	TrimmedSpace GetTrimmedMesh(int Udivisions, int Vdivisions);
protected:
	IntersectionData m_intersectionData;
};