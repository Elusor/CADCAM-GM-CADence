#include "IntersectionCurve.h"
#include "imgui.h"
#include "Scene.h"
#include "Node.h"
#include "IParametricSurface.h"

IntersectionCurve::IntersectionCurve()
{
}

void IntersectionCurve::Initialize(
	ObjectRef qSurface, std::vector<DirectX::XMFLOAT2> qParameters, bool qIsLooped,
	ObjectRef pSurface, std::vector<DirectX::XMFLOAT2> pParameters, bool pIsLooped)
{
	m_qSurface = qSurface;
	m_pSurface = pSurface;

	m_qParameters = qParameters;
	m_pParameters = pParameters;

	m_qIsLooped = qIsLooped;
	m_pIsLooped = pIsLooped;

	m_positions = GetPointPositions();	
	GetInterpolationSplineBernsteinPoints();
	CountPointsOnBorders(qSurface, qParameters, pSurface, pParameters);
	for (auto pt : m_virtualPoints)
	{
		m_virtualPointsWeak.push_back(pt);
	}

	SetModified(true);
}

ObjectRef IntersectionCurve::GetParametricSurface(IntersectedSurface surface)
{
	auto res = surface == SurfaceP ? m_pSurface : m_qSurface;
	return res;
}

bool CheckIsClosed(ObjectRef surface, int uMax, int uZero, int vMax, int vZero, bool isLooped)
{
	bool res = false;
	if (auto surNode = surface.lock())
	{
		auto surface = dynamic_cast<IParametricSurface*>(surNode->m_object.get());
		if (isLooped && (uMax + uZero + vMax + vZero) % 4 == 0)
		{
			res = isLooped;
		}
		else
		{
			bool borderClosedU = false;
			bool borderClosedV = false;

			if (surface->IsWrappedInDirection(SurfaceWrapDirection::None) &&
				uMax + uZero + vMax + vZero > 1)
			{
				res = true;
			}

			//SurfaceWrapDirection::Height
			if (surface->IsWrappedInDirection(SurfaceWrapDirection::Width) == false &&
				uMax % 2 == 0 && uMax != 0 &&
				uZero % 2 == 0 && uZero != 0)
			{
				res = true;
			}

			if (surface->IsWrappedInDirection(SurfaceWrapDirection::Width) &&
				surface->IsWrappedInDirection(SurfaceWrapDirection::Height) == false &&
				uMax % 2 == 1 && uMax != 0 &&
				uZero % 2 == 1 && uZero != 0)
			{
				res = true;
			}

			//SurfaceWrapDirection::Width
			if (surface->IsWrappedInDirection(SurfaceWrapDirection::Height) == false &&
				vMax % 2 == 0 && vMax != 0 &&
				vZero % 2 == 0 && vZero != 0)
			{
				res = true;
			}

			if (surface->IsWrappedInDirection(SurfaceWrapDirection::Height)&&
				surface->IsWrappedInDirection(SurfaceWrapDirection::Width) == false &&
				vMax % 2 == 1 && vMax != 0 &&
				vZero % 2 == 1 && vZero != 0)
			{
				res = true;
			}

		}

	}
	return res;
}

bool IntersectionCurve::GetIsClosedIntersection(IntersectedSurface surface)
{

	bool isClosed = false;
	switch (surface)
	{
	case IntersectedSurface::SurfaceQ:	
		isClosed = CheckIsClosed(
			GetParametricSurface(surface), 
			m_uPtsOnBorderMax, m_uPtsOnBorderZero, 
			m_vPtsOnBorderMax, m_vPtsOnBorderZero, m_qIsLooped);
		break;
	case IntersectedSurface::SurfaceP:
		isClosed = CheckIsClosed(
			GetParametricSurface(surface),
			m_sPtsOnBorderMax, m_sPtsOnBorderZero,
			m_tPtsOnBorderMax, m_tPtsOnBorderZero, m_pIsLooped);
		break;
	}

	return isClosed;
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

std::vector<DirectX::XMFLOAT2> IntersectionCurve::GetNormalizedParameterList(IntersectedSurface surface)
{
	auto surfRef = surface == IntersectedSurface::SurfaceQ ? m_qSurface : m_pSurface;
	auto originalParams = surface == IntersectedSurface::SurfaceQ ? m_qParameters : m_pParameters;

	std::vector<DirectX::XMFLOAT2> normalizedParams;

	if (auto surfNode = surfRef.lock())
	{
		auto surf = dynamic_cast<IParametricSurface*>(surfNode->m_object.get());
		for (auto paramPair : originalParams)
		{
			auto normalized = surf->GetNormalizedParams(paramPair.x, paramPair.y);
			normalizedParams.push_back({ normalized.u, normalized.v });
		}
	}
	

	return normalizedParams;
}

std::vector<DirectX::XMFLOAT3> IntersectionCurve::GetPointPositions()
{
	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<DirectX::XMFLOAT3> positions2;

	if(m_qSurface.expired() == false && m_pSurface.expired() == false)
	{ 
		auto qNode = m_qSurface.lock();
		auto pNode = m_pSurface.lock();

		auto qSurf = dynamic_cast<IParametricSurface*>(qNode->m_object.get());
		auto pSurf = dynamic_cast<IParametricSurface*>(pNode->m_object.get());

		for (auto params : m_qParameters)
		{
			positions.push_back(qSurf->GetPoint(params.x, params.y));
		}

		for (auto params : m_pParameters)
		{
			positions2.push_back(pSurf->GetPoint(params.x, params.y));
		}
	}

	return positions;
}

void IntersectionCurve::RenderObjectSpecificContextOptions(Scene& scene)
{
	if (!m_qSurface.expired() && !m_pSurface.expired())
	{
		if (!GetIsClosedIntersection(SurfaceQ) && !GetIsClosedIntersection(SurfaceP))
		{
			ImGui::Text("Cannot trim");
		}
		else {
			if (ImGui::Selectable("Trim"))
			{
				TrimAffectedSurfaces();
			}
		}
		

		if (ImGui::Selectable("Visualize in parameter space"))
		{
			scene.m_curveVisualizer->VisualizeCurve(this->m_nodePtr);
		}
	}


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

void IntersectionCurve::UpdateObject()
{	
	if (m_virtualPoints.size() >= 2)
	{ 
		UpdateGSData();
	}
}

void IntersectionCurve::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	auto controlPoints = m_virtualPointsWeak;
	if (controlPoints.size() >= 2)
	{
		CalculateAdaptiveRendering(controlPoints, renderState);
		RenderCurve(renderState);
	}
}

bool IntersectionCurve::CreateParamsGui()
{
	ImGui::Begin("Inspector");
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());
	ImGui::Text("Samples: ");
	ImGui::SameLine(); ImGui::Text(std::to_string(m_adaptiveRenderingSamples).c_str());

	ImGui::Spacing();
	bool objectChanged = false;

	// change colors for the curve
	float mcolor[3] = {
		m_meshDesc.m_adjustableColor.x,
		m_meshDesc.m_adjustableColor.y,
		m_meshDesc.m_adjustableColor.z,
	};

	std::string mtext = "Curve color";
	ImGui::Text(mtext.c_str());
	objectChanged |= ImGui::ColorEdit3(("##" + mtext + GetIdentifier()).c_str(), (float*)&mcolor);

	m_meshDesc.m_adjustableColor.x = mcolor[0];
	m_meshDesc.m_adjustableColor.y = mcolor[1];
	m_meshDesc.m_adjustableColor.z = mcolor[2];
	ImGui::Spacing();		

	ImGui::End();
	return objectChanged;
}

bool IntersectionCurve::GetIsModified()
{
	return m_modified;
}

std::shared_ptr<Node> IntersectionCurve::CreateVirtualPoint(DirectX::XMFLOAT3 pos, int ptIdx)
{
	Point* p = new Point();
	p->m_defaultName = p->m_name = "Bernstein point " + std::to_string(ptIdx);
	p->SetPosition(pos);

	Node* node = new Node();
	node->SetIsVirtual(true);
	node->m_object = std::unique_ptr<Point>(p);
	// fill m_curBasisControlPoints with Bernstein points - that is points calculated from m_virtualBernsteinPoints	
	std::shared_ptr<Node> nodeptr = std::shared_ptr<Node>(node);
	return nodeptr;
}

void IntersectionCurve::GetInterpolationSplineBernsteinPoints()
{

	if (m_positions.size() < 2)
	{
		m_virtualPoints.clear();
		return;
	}


	std::vector<float> upperDiag, diag, lowerDiag;
	std::vector<float> xVector, yVector, zVector;
	std::vector<float> distances;

	std::vector<DirectX::XMFLOAT3> a, b, c, d;

	for (int i = 0; i < m_positions.size() - 1; i++)
	{
		auto p1 = m_positions[i];
		auto p2 = m_positions[i + 1];
		// DISTANCES FROM d0 to dk-1, where k is knots count
		float dist = GetDistanceBetweenPoints(p2, p1);
		dist = max(dist, 0.001f);
		distances.push_back(dist);
	}

	for (int i = 0; i < m_positions.size() - 2; i++)
	{
		diag.push_back(2.0f);
	}

	for (int i = 1; i < m_positions.size() - 1; i++)
	{
		float dist = distances[i];
		float distPrev = distances[i - 1];
		float denom = dist + distPrev;

		// calculate R's alphas (lower diag) and betas (upper diag)				
		if (i == 1)
		{
			lowerDiag.push_back(0.f);
		}
		else
		{
			float alphai = distances[i - 1]; //TODO
			alphai /= denom;
			lowerDiag.push_back(alphai);
		}

		if (i == m_positions.size() - 2)
		{
			upperDiag.push_back(0.f);
		}
		else
		{
			float betai = distances[i]; //TODO
			betai /= denom;
			upperDiag.push_back(betai);
		}

		// calculate and assing Ri
		auto P = m_positions[i];
		auto Pnext = m_positions[i + 1];
		auto Pprev = m_positions[i - 1];

		auto diff = XMF3TimesFloat(XMF3SUB(Pnext, P), 1.f / dist);
		auto diffPrev = XMF3TimesFloat(XMF3SUB(P, Pprev), 1.f / distPrev);
		auto R = XMF3SUB(diff, diffPrev);
		R = XMF3TimesFloat(R, 3.f / denom);
		xVector.push_back(R.x);
		yVector.push_back(R.y);
		zVector.push_back(R.z);
	}

	std::vector<float> xRes, yRes, zRes;

	if (xVector.size() > 1)
	{
		xRes = SolveTridiagMatrix(lowerDiag, diag, upperDiag, xVector);
		yRes = SolveTridiagMatrix(lowerDiag, diag, upperDiag, yVector);
		zRes = SolveTridiagMatrix(lowerDiag, diag, upperDiag, zVector);
	}
	else
	{
		if (xVector.size() == 1)
		{
			xRes.push_back(xVector[0] / 2.f);
			yRes.push_back(yVector[0] / 2.f);
			zRes.push_back(zVector[0] / 2.f);
		}
	}

	std::vector<DirectX::XMFLOAT3> resultPos;
	if (m_positions.size() > 2)
	{
		DirectX::XMFLOAT3 zero = { 0.f, 0.f, 0.f };

		auto p0 = m_positions[0];
		auto p1 = m_positions[1];

		c.push_back(zero);
		a.push_back(p0);
		//Xres size is K - 2
		for (int i = 0; i < xRes.size(); i++)
		{
			c.push_back(DirectX::XMFLOAT3(xRes[i], yRes[i], zRes[i]));
			a.push_back(m_positions[i + 1]);
			//d.push_back(interpolationKnots[i + 2].lock()->m_object->GetPosition());
		}

		auto plast = m_positions[m_positions.size() - 1];
		c.push_back(zero);
		a.push_back(plast);

		// d can be shorter by 1 than a
		//calculate d's
		for (int i = 1; i < c.size(); i++)
		{
			float distPrev = distances[i - 1];
			XMFLOAT3 cDiff = XMF3SUB(c[i], c[i - 1]);
			XMFLOAT3 dmod = XMF3TimesFloat(cDiff, 1.f / (3.f * distPrev));
			d.push_back(dmod);
		}

		for (int i = 1; i < c.size(); i++)
		{
			float distPrev = distances[i - 1];
			float dpSqr = distPrev * distPrev;
			float dpCube = dpSqr * distPrev;
			XMFLOAT3 bmod;
			XMFLOAT3 aDiff = XMF3SUB(a[i], a[i - 1]);
			XMFLOAT3 dModified = XMF3TimesFloat(d[i - 1], dpCube);
			XMFLOAT3 cModified = XMF3TimesFloat(c[i - 1], dpSqr);
			bmod = XMF3SUB(aDiff, dModified);
			bmod = XMF3SUB(bmod, cModified);
			bmod = XMF3TimesFloat(bmod, 1.f / distPrev);
			b.push_back(bmod);
		}

		for (int i = 0; i < b.size(); i++)
		{
			float dist = distances[i];
			a[i] = XMF3TimesFloat(a[i], 1.f);
			b[i] = XMF3TimesFloat(b[i], dist);
			c[i] = XMF3TimesFloat(c[i], dist * dist);
			d[i] = XMF3TimesFloat(d[i], dist * dist * dist);

		}

		// convert abcd to bernstein basis
		for (int i = 0; i < b.size(); i++)
		{
			DirectX::XMFLOAT4X4 resMat;
			DirectX::XMFLOAT4X4 mtx = {
				a[i].x, b[i].x, c[i].x, d[i].x,
				a[i].y, b[i].y, c[i].y, d[i].y,
				a[i].z, b[i].z, c[i].z, d[i].z,
				0.f, 0.f, 0.f, 0.f
			};

			DirectX::XMMATRIX vectorMat = DirectX::XMLoadFloat4x4(&mtx);

			auto res = vectorMat * DirectX::XMLoadFloat4x4(&m_changeBasisMtx);

			DirectX::XMStoreFloat4x4(&resMat, (res));
			auto k1 = XMFLOAT3(resMat._11, resMat._21, resMat._31);
			auto k2 = XMFLOAT3(resMat._12, resMat._22, resMat._32);
			auto k3 = XMFLOAT3(resMat._13, resMat._23, resMat._33);
			auto k4 = XMFLOAT3(resMat._14, resMat._24, resMat._34);

			resultPos.push_back(k1);
			resultPos.push_back(k2);
			resultPos.push_back(k3);
		}

		resultPos.push_back(m_positions[m_positions.size() - 1]);
		// Each segmenent is built from 4 points, the middle points are the same
	}
	else
	{
		XMFLOAT3 pos0 = m_positions[0];
		XMFLOAT3 pos1 = m_positions[1];

		resultPos.push_back(pos0);
		resultPos.push_back(F3BadLerp(pos0, pos1, 1.f / 3.f));
		resultPos.push_back(F3BadLerp(pos0, pos1, 2.f / 3.f));
		resultPos.push_back(pos1);
	}

	int pointCount = resultPos.size();

	if (m_virtualPoints.size() == pointCount)
	{
		//update values
		for (int i = 0; i < resultPos.size(); i++)
		{
			auto point = m_virtualPoints[i];
			point->m_object->SetPosition(resultPos[i]);
		}
	}
	else
	{
		m_virtualPoints.clear();
		for (int i = 0; i < resultPos.size(); i++)
		{
			m_virtualPoints.push_back(CreateVirtualPoint(resultPos[i], i));
		}
	}
}

void IntersectionCurve::UpdateGSData()
{
	m_lastVertexDuplicationCount = 0;
	std::vector<VertexPositionColor> vertices;
	std::vector<unsigned short> indices;

	for (int i = 0; i < m_virtualPoints.size(); i++)
	{
		//add all vertices
		auto pos = m_virtualPoints[i]->m_object->GetPosition();
		vertices.push_back(VertexPositionColor{
					pos,
					m_meshDesc.m_defaultColor });
	}

	for (int i = 0; i < m_virtualPoints.size(); i += 3)
	{
		// mage edges out of the vertices
		// There are some elements that should be added
		int remVerts = m_virtualPoints.size() - 1 - i;
		if (remVerts > 0)
		{
			// add next 4 points normally
			if (m_virtualPoints.size() - i >= 4)
			{
				indices.push_back(i);
				indices.push_back(i + 1);
				indices.push_back(i + 2);
				indices.push_back(i + 3);
			}
			// add the rest of the nodes and add the last node multiple times
			else {
				for (int j = i; j < m_virtualPoints.size(); j++)
				{
					indices.push_back(j);
				}

				// add the rest of the vertices as duplicates of the last one
				int emptyVertices = 4 - (m_virtualPoints.size() - i);
				//m_lastVertexDuplicationCount = emptyVertices;
				for (int k = 0; k < emptyVertices; k++)
				{
					indices.push_back(m_virtualPoints.size() - 1);
				}
			}

		}
	}

	m_meshDesc.vertices = vertices;
	m_meshDesc.indices = indices;
	m_meshDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
}

void IntersectionCurve::TrimAffectedSurfaces()
{
	if(GetIsClosedIntersection(SurfaceQ))
		TrimSurface(IntersectedSurface::SurfaceQ);

	if(GetIsClosedIntersection(SurfaceP))
		TrimSurface(IntersectedSurface::SurfaceP);	
}

void IntersectionCurve::TrimSurface(IntersectedSurface surface)
{
	auto curveRef = m_nodePtr;
	auto surfRef = GetParametricSurface(surface);	
	bool surfValidForTrimming = surfRef.expired() == false;

	if(surfValidForTrimming)
	{
		auto surfNode = surfRef.lock();
		auto surf = dynamic_cast<IParametricSurface*>(surfNode->m_object.get());

		try {
			IntersectionData data;
			data.affectedSurface = surface;
			data.intersectionCurve = curveRef;
			auto intersectable = dynamic_cast<TrimmableSurface*>(surf);
			if (intersectable != nullptr)
				intersectable->SetIntersectionData(data);

			auto object = dynamic_cast<Object*>(surf);
			if (object != nullptr)
				object->SetModified(true);
		}
		catch (std::bad_cast bc)
		{
			assert(false && "Invalid surface. Could not trim.");
		}
	}
}

void IntersectionCurve::CountPointsOnBorders(
	ObjectRef qSurface, std::vector<DirectX::XMFLOAT2> qParameters, 
	ObjectRef pSurface, std::vector<DirectX::XMFLOAT2> pParameters) 
{
	m_uPtsOnBorderMax = 0;
	m_uPtsOnBorderZero = 0;

	m_vPtsOnBorderMax = 0;
	m_vPtsOnBorderZero = 0;

	m_sPtsOnBorderMax = 0;
	m_sPtsOnBorderZero = 0;

	m_tPtsOnBorderMax = 0;
	m_tPtsOnBorderZero = 0;


	if (auto qNode = qSurface.lock())
	{
		auto qSurf = dynamic_cast<IParametricSurface*>(qNode->m_object.get());
		ParameterPair qMaxParams = qSurf->GetMaxParameterValues();

		for (int i = 0; i < qParameters.size(); i++)
		{
			ParameterPair pair = qParameters[i];
			if (pair.u == qMaxParams.u)
			{
				m_uPtsOnBorderMax++;
			}
			if (pair.u == 0)
			{
				m_uPtsOnBorderZero++;
			}
			if (pair.v == qMaxParams.v)
			{
				m_vPtsOnBorderMax++;
			}			
			if (pair.v == 0)
			{
				m_vPtsOnBorderZero++;
			}
		}
	}

	if (auto pNode = pSurface.lock())
	{
		auto pSurf = dynamic_cast<IParametricSurface*>(pNode->m_object.get());
		ParameterPair pMaxParams = pSurf->GetMaxParameterValues();

		for (int i = 0; i < pParameters.size(); i++)
		{
			ParameterPair pair = pParameters[i];
			if (pair.u == pMaxParams.u)
			{
				m_sPtsOnBorderMax++;
			}
			if (pair.u == 0)
			{
				m_sPtsOnBorderZero++;
			}
			if (pair.v == pMaxParams.v)
			{
				m_tPtsOnBorderMax++;
			}
			if (pair.v == 0)
			{
				m_tPtsOnBorderZero++;
			}
		}
	}


}
