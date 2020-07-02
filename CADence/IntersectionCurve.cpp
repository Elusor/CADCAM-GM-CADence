#include "IntersectionCurve.h"
#include "imgui.h"
#include "Scene.h"
#include "Node.h"

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

void IntersectionCurve::RenderObjectSpecificContextOptions(Scene& scene)
{
	if (ImGui::Selectable("Convert to interpolation curve"))
	{
		if (auto node = m_nodePtr.lock())
		{
			auto factory = scene.m_objectFactory.get();
			auto positions = m_positions;

			std::vector<std::weak_ptr<Node>> points;
			for (auto pos : positions)
			{
				// Create point and attach it to the scene
				auto pt = factory->CreatePoint();
				pt->m_object->SetPosition(pos);
				scene.AttachObject(pt);
				points.push_back(pt);
			}

			auto interpolCurve = factory->CreateInterpolBezierCurveC2(points);
			// Optionally rename the curve to mark that this is the same object
			interpolCurve->Rename(m_name);
			scene.RemoveObject(node->m_object);
			scene.AttachObject(interpolCurve);
		}
	}
}
