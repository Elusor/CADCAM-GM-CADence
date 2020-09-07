#pragma once
#include "IntersectionData.h"
#include "Trimmer.h"

class TrimmableSurface
{
public:
	SampleInOutStatus GetCurrentTrimSide();
	TrimmedSpace GetTrimmedMesh(int Udivisions, int Vdivisions);

	void SetIntersectionData(IntersectionData data);
	void SwitchTrimmedSide();	

protected:

	SampleInOutStatus m_currentTrimSide;
	IntersectionData m_intersectionData;
	bool m_trimSidesSwitched;

	bool CreateTrimSwitchGui(std::string identifier);
};