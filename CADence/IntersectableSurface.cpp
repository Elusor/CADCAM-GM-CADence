#include "IntersectableSurface.h"

SampleInOutStatus TrimmableSurface::GetCurrentTrimSide()
{
	return m_currentTrimSide;
}

TrimmedSpace TrimmableSurface::GetTrimmedMesh(int uDivisions, int vDivisions)
{
	TrimmedSpace space;
	if (auto node = m_intersectionData.intersectionCurve.lock())
	{
		auto curve = dynamic_cast<IntersectionCurve*>(node->m_object.get());
		auto paramList = curve->GetNormalizedParameterList(m_intersectionData.affectedSurface);
		space = Trimmer::Trim(paramList, uDivisions, vDivisions, GetCurrentTrimSide());
	}
	return space;
}

void TrimmableSurface::SetIntersectionData(IntersectionData data)
{
	m_intersectionData = data;
}

void TrimmableSurface::SwitchTrimmedSide()
{
	m_currentTrimSide = Trimmer::GetOppositeStatus(m_currentTrimSide);
}

bool TrimmableSurface::CreateTrimSwitchGui(std::string identifier)
{
	std::string trimSide = "Switch trimmed side##" + identifier;
	bool trimChanged = ImGui::Checkbox(trimSide.c_str(), &m_trimSidesSwitched);
	if (trimChanged)
	{
		SwitchTrimmedSide();
	}
	return trimChanged;
}
