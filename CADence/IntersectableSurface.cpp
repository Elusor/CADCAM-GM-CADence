#include "IntersectableSurface.h"

void IntersectableSurface::SetIntersectionData(IntersectionData data)
{
	m_intersectionData = data;
}

TrimmedSpace IntersectableSurface::GetTrimmedMesh(int uDivisions, int vDivisions)
{
	TrimmedSpace space;
	if (auto node = m_intersectionData.intersectionCurve.lock())
	{
		auto curve = dynamic_cast<IntersectionCurve*>(node->m_object.get());
		auto paramList = curve->GetNormalizedParameterList(m_intersectionData.affectedSurface);
		space = Trimmer::Trim(paramList, uDivisions, vDivisions);
	}
	return space;
}
