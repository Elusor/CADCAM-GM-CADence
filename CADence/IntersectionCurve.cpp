#include "IntersectionCurve.h"

IntersectionCurve::IntersectionCurve()
{
}

void IntersectionCurve::Initialize(IParametricSurface* qSurface, std::vector<DirectX::XMFLOAT2> qParameters, IParametricSurface* pSurface, std::vector<DirectX::XMFLOAT2> pParameters)
{
	m_qSurface = qSurface;
	m_pSurface = pSurface;

	m_qParameters = qParameters;
	m_pParameters = pParameters;

	m_positions = GetPointPositions();
}

std::vector<DirectX::XMFLOAT2> IntersectionCurve::GetParameterList(IntersectedSurface surface)
{
	std::vector<DirectX::XMFLOAT2> paramsList;
	switch (surface)
	{
	case IntersectedSurface::SurfaceQ:
		paramsList = m_qParameters;
		break;
	case IntersectedSurface::SurfaceP:
		paramsList = m_pParameters;
		break;
	}

	return paramsList;
}

std::vector<DirectX::XMFLOAT3> IntersectionCurve::GetPointPositions()
{
	std::vector<DirectX::XMFLOAT3> positions;
	for (auto params : m_pParameters)
	{
		positions.push_back(m_pSurface->GetPoint(params.x, params.y));
	}

	return positions;
}
