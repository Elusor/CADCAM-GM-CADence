#include "IntersectableSurface.h"

std::vector<XMFLOAT2> RemoveAdjacentDuplicatesInList(std::vector<XMFLOAT2> list)
{
	std::vector<XMFLOAT2> listWithoutDuplicates;
	XMFLOAT2 lastParams = list[0];
	listWithoutDuplicates.push_back(lastParams);
	for (int i = 1; i < list.size(); i++)
	{
		auto curParams = list[i];
		if (curParams.x != lastParams.x || curParams.y != lastParams.y)
		{
			lastParams = curParams;
			listWithoutDuplicates.push_back(curParams);
		}
	}

	return listWithoutDuplicates;
}

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
		auto paramList = RemoveAdjacentDuplicatesInList(curve->GetNormalizedParameterList(m_intersectionData.affectedSurface));
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
	bool trimChanged = false;
	if (auto curveNode = m_intersectionData.intersectionCurve.lock())
	{
		auto curve = dynamic_cast<IntersectionCurve*>(curveNode->m_object.get());
		if (curve->GetIsClosedIntersection(m_intersectionData.affectedSurface))
		{
			std::string trimSide = "Switch trimmed side##" + identifier;
			trimChanged = ImGui::Checkbox(trimSide.c_str(), &m_trimSidesSwitched);
			if (trimChanged)
			{
				SwitchTrimmedSide();
			}
		}
	}
	
	return trimChanged;
}
