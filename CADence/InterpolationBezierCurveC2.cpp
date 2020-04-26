#include "InterpolationBezierCurveC2.h"
#include "GroupNode.h"

using namespace DirectX;
DirectX::XMFLOAT4X4 changeBasisMtx = {
		1.f, 1.f, 1.f, 1.f,
		0.f,  1.f / 3.f, 2.f / 3.f, 1.f,
		0.f, 0.f, 1.f / 3.f, 1.f,
		0.f, 0.f, 0.f, 1.f };

InterpolationBezierCurveC2::InterpolationBezierCurveC2(): InterpolationBezierCurveC2(std::vector<std::weak_ptr<Node>>())
{	
}

InterpolationBezierCurveC2::InterpolationBezierCurveC2(std::vector<std::weak_ptr<Node>> initialKnots)
{
	m_adaptiveRenderingSamples = 0;
	m_renderPolygon = false;
	m_controlPoints = initialKnots;
	if (initialKnots.size() > 0)
		GetInterpolationSplineBernsteinPoints(m_controlPoints);
}

void InterpolationBezierCurveC2::UpdateObject()
{
	
	if (m_controlPoints.size() >= 2)
	{
		RecalculateIfModified();

		UpdateGSData();
		PreparePolygonDesc();
	}
}

bool InterpolationBezierCurveC2::CreateParamsGui()
{
	ImGui::Begin("Inspector");
	// Create sliders for torus parameters	
	ImGui::Text("Name: ");
	ImGui::SameLine(); ImGui::Text(m_name.c_str());
	ImGui::Text("Samples: ");
	ImGui::SameLine(); ImGui::Text(std::to_string(m_adaptiveRenderingSamples).c_str());


	ImGui::Spacing();
	bool objectChanged = false;

	// checkbox for deBoore points

	// change colors for the curve
	float mcolor[3] = {
		m_meshDesc.m_defaultColor.x,
		m_meshDesc.m_defaultColor.y,
		m_meshDesc.m_defaultColor.z,
	};

	std::string mtext = "Curve color";
	ImGui::Text(mtext.c_str());
	objectChanged |= ImGui::ColorEdit3(("##" + mtext + GetIdentifier()).c_str(), (float*)&mcolor);

	m_meshDesc.m_defaultColor.x = mcolor[0];
	m_meshDesc.m_defaultColor.y = mcolor[1];
	m_meshDesc.m_defaultColor.z = mcolor[2];
	ImGui::Spacing();

	// checkbox for Bernstein's polygon
	std::string label = "Display polygon" + GetIdentifier();
	objectChanged |= ImGui::Checkbox(label.c_str(), &m_renderPolygon);
	ImGui::Spacing();

	// change colors for polygon
	float pcolor[3] = {
		m_PolygonDesc.m_defaultColor.x,
		m_PolygonDesc.m_defaultColor.y,
		m_PolygonDesc.m_defaultColor.z,
	};

	std::string ptext = "Polygon color";
	ImGui::Text(ptext.c_str());
	objectChanged |= ImGui::ColorEdit3(("##" + ptext + GetIdentifier()).c_str(), (float*)&pcolor);

	m_PolygonDesc.m_defaultColor.x = pcolor[0];
	m_PolygonDesc.m_defaultColor.y = pcolor[1];
	m_PolygonDesc.m_defaultColor.z = pcolor[2];
	ImGui::Spacing();

	ImGui::End();
	return objectChanged;
}

void InterpolationBezierCurveC2::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	if (m_controlPoints.size() >= 2)
	{
		CalculateAdaptiveRendering(m_controlPoints, renderState);
		RenderCurve(renderState);
		RenderPolygon(renderState);
	}
}

void InterpolationBezierCurveC2::AttachChild(std::weak_ptr<Node> controlPoint)
{
	m_controlPoints.push_back(controlPoint);	
	SetModified(true);
}

void InterpolationBezierCurveC2::RemoveChild(std::weak_ptr<Node> controlPoint)
{
	if (auto controlPt = controlPoint.lock())
	{
		auto it = m_controlPoints.begin();
		while (it != m_controlPoints.end())
		{
			if (auto node = it->lock())
			{
				if (node == controlPt)
				{
					it = m_controlPoints.erase(it);
				}
				else {
					it++;
				}
			}
			else {
				it = m_controlPoints.erase(it);
			}
		}
	}
	SetModified(true);
}

bool InterpolationBezierCurveC2::GetIsModified()
{

	for (int i = 0; i < m_controlPoints.size(); i++)
	{
		if (auto point = m_controlPoints[i].lock())
		{			
			if (point->m_object->GetIsModified())
			{
				SetModified(true);
			}
		}
	}
	return m_modified;
}

bool InterpolationBezierCurveC2::RemoveExpiredChildren()
{
	bool removed = false;
	auto it = m_controlPoints.begin();
	while (it != m_controlPoints.end())
	{
		if (auto pt = it->lock())
		{
			it++;
		}
		else {
			it = m_controlPoints.erase(it);
			removed = true;
		}
	}

	if (auto parent = m_parent.lock())
	{
		auto gParent = dynamic_cast<GroupNode*>(parent.get());
		gParent->RemoveExpiredChildren();
	}

	return removed;
}

void InterpolationBezierCurveC2::RecalculateIfModified()
{
	if (RemoveExpiredChildren())
	{
		//Recalculate interpolation Points
		GetInterpolationSplineBernsteinPoints(m_controlPoints);
	}

	bool isModified = m_modified;
	for (int i = 0; i < m_controlPoints.size(); i++)
	{
		isModified |= m_controlPoints[i].lock()->m_object->GetIsModified();
	}

	if (isModified)
	{
		GetInterpolationSplineBernsteinPoints(m_controlPoints);
	}
}

void InterpolationBezierCurveC2::GetInterpolationSplineBernsteinPoints(std::vector<std::weak_ptr<Node>> interpolationKnots)
{
	std::vector<float> upperDiag, diag, lowerDiag;
	std::vector<float> xVector, yVector, zVector;
	std::vector<float> distances;

	std::vector<DirectX::XMFLOAT3> a, b, c, d;

	for (int i = 0; i < interpolationKnots.size() - 1; i++)
	{
		auto p1 = interpolationKnots[i].lock()->m_object->GetPosition();
		auto p2 = interpolationKnots[i + 1].lock()->m_object->GetPosition();
		// DISTANCES FROM d0 to dk-1, where k is knots count
		distances.push_back(GetDistanceBetweenPoints(p2, p1));
	}

	float L = 0;
	for (int i = 0; i < distances.size(); i++)
	{
		L += distances[i];
	}

	for (int i = 0; i < interpolationKnots.size() - 2; i++)
	{
		diag.push_back(2.0f);
	}

	for (int i = 1; i < interpolationKnots.size() - 1; i++)
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

		if (i == interpolationKnots.size() - 2)
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
		auto P = interpolationKnots[i].lock()->m_object->GetPosition();;
		auto Pnext = interpolationKnots[i + 1].lock()->m_object->GetPosition();;
		auto Pprev = interpolationKnots[i - 1].lock()->m_object->GetPosition();;

		auto diff = XMF3TimesFloat(XMF3SUB(Pnext, P), 1.f / dist);
		auto diffPrev = XMF3TimesFloat(XMF3SUB(P, Pprev), 1.f / distPrev);
		auto R = XMF3SUB(diff, diffPrev);
		R = XMF3TimesFloat(R, 3.f / denom);
		xVector.push_back(R.x);
		yVector.push_back(R.y);
		zVector.push_back(R.z);
	}

	/*
	uniform knots
	lowerDiag.push_back(0.f);
	for (int i = 0; i < interpolationKnots.size() - 1; i++)
	{
		upperDiag.push_back(1.0f);
		lowerDiag.push_back(1.0f);
	}
	upperDiag.push_back(0.f);
	// insert the first point

	// remove this
	auto p0 = interpolationKnots[0].lock();
	auto p1 = interpolationKnots[1].lock();
	auto pnPos = p1->m_object->GetPosition();
	auto pnPosPrev = p0->m_object->GetPosition();
	auto diff = XMF3SUB(pnPos, pnPosPrev);
	xVector.push_back(3.f * diff.x);
	yVector.push_back(3.f * diff.y);
	zVector.push_back(3.f * diff.z);

	for (int i = 0; i < interpolationKnots.size() - 2; i++)
	{
		auto pnPos = interpolationKnots[i].lock()->m_object->GetPosition();
		auto pnPosNext = interpolationKnots[i + 2].lock()->m_object->GetPosition();
		auto diffr = XMF3SUB(pnPosNext, pnPos);
		xVector.push_back(3.f * diffr.x);
		yVector.push_back(3.f * diffr.y);
		zVector.push_back(3.f * diffr.z);

	}
	// insert the last point
	auto pnprev = interpolationKnots[interpolationKnots.size() - 2].lock();
	auto pn = interpolationKnots[interpolationKnots.size() - 1].lock();
	auto pnPos2 = pn->m_object->GetPosition();
	auto pnPosPrev2 = pnprev->m_object->GetPosition();
	auto diff3 = XMF3SUB(pnPos2, pnPosPrev2);
	xVector.push_back(3.f * diff3.x);
	yVector.push_back(3.f * diff3.y);
	zVector.push_back(3.f * diff3.z);
	*/

	auto xRes = SolveTridiagMatrix(lowerDiag, diag, upperDiag, xVector);
	auto yRes = SolveTridiagMatrix(lowerDiag, diag, upperDiag, yVector);
	auto zRes = SolveTridiagMatrix(lowerDiag, diag, upperDiag, zVector);

	DirectX::XMFLOAT3 zero = { 0.f, 0.f, 0.f };

	auto p0 = interpolationKnots[0].lock()->m_object->GetPosition();
	auto p1 = interpolationKnots[1].lock()->m_object->GetPosition();

	c.push_back(zero);
	a.push_back(p0);
	//Xres size is K - 2
	for (int i = 0; i < xRes.size(); i++)
	{
		c.push_back(DirectX::XMFLOAT3(xRes[i], yRes[i], zRes[i]));
		a.push_back(interpolationKnots[i + 1].lock()->m_object->GetPosition());
		//d.push_back(interpolationKnots[i + 2].lock()->m_object->GetPosition());
	}

	auto plast = interpolationKnots[interpolationKnots.size() - 1].lock()->m_object->GetPosition();
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
		float dist= distances[i];
		a[i] = XMF3TimesFloat(a[i], 1.f);
		b[i] = XMF3TimesFloat(b[i], dist);
		c[i] = XMF3TimesFloat(c[i], dist*dist);
		d[i] = XMF3TimesFloat(d[i], dist*dist*dist);

	}

	//// TODO CHANGE THIS TO REPRESENT PROPER coeffs
	////calc a b c d for each res
	//for (int i = 0; i < xRes.size() - 1; i++)
	//{	
	//	/*
	//	uniform knots coeffs
	//	auto yi = interpolationKnots[i].lock()->m_object->GetPosition();
	//	auto yi1 = interpolationKnots[i+1].lock()->m_object->GetPosition();

	//	//diff = (yi+1 - yi)		
	//	auto diff4 = XMF3SUB(yi1,yi);
	//	// Di
	//	auto Di = DirectX::XMFLOAT3(xRes[i], yRes[i], zRes[i]);
	//	// Di+1
	//	auto Di1 = DirectX::XMFLOAT3(xRes[i + 1], yRes[i + 1], zRes[i + 1]);
	//	// a = y
	//	a.push_back(yi);
	//	// b = Di
	//	b.push_back(Di);
	//	// c = 3 diff - 2Di - Di+1
	//	c.push_back(XMF3TimesFloat(diff4, 3.f));
	//	c[i] = XMF3SUB(c[i], XMF3TimesFloat(Di, 2.f));
	//	c[i] = XMF3SUB(c[i], Di1);
	//	// d = -2 diff + Di + Di+1
	//	d.push_back(XMF3TimesFloat(diff4, -2.f));
	//	d[i] = XMF3SUM(d[i], Di);
	//	d[i] = XMF3SUM(d[i], Di1);
	//	*/
	//}	


	std::vector<DirectX::XMFLOAT3> resultPos;

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

		auto res = vectorMat * DirectX::XMLoadFloat4x4(&changeBasisMtx);

		DirectX::XMStoreFloat4x4(&resMat, (res));
		auto k1 = XMFLOAT3(resMat._11, resMat._21, resMat._31);
		auto k2 = XMFLOAT3(resMat._12, resMat._22, resMat._32);
		auto k3 = XMFLOAT3(resMat._13, resMat._23, resMat._33);
		auto k4 = XMFLOAT3(resMat._14, resMat._24, resMat._34);

		resultPos.push_back(k1);
		resultPos.push_back(k2);
		resultPos.push_back(k3);
	}

	resultPos.push_back(interpolationKnots[interpolationKnots.size() - 1].lock()->m_object->GetPosition());
	// Each segmenent is built from 4 points, the middle points are the same

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

std::shared_ptr<Node> InterpolationBezierCurveC2::CreateVirtualPoint(DirectX::XMFLOAT3 pos, int ptIdx)
{
	Point* p = new Point();
	p->m_defaultName = p->m_name = "Bernstein point " + std::to_string(ptIdx);
	p->SetPosition(pos);

	Node* node = new Node();
	node->m_isVirtual = true;
	node->m_object = std::unique_ptr<Point>(p);
	// fill m_curBasisControlPoints with Bernstein points - that is points calculated from m_virtualBernsteinPoints	
	std::shared_ptr<Node> nodeptr = std::shared_ptr<Node>(node);
	return nodeptr;
}

void InterpolationBezierCurveC2::UpdateGSData()
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
#ifdef DEBUG
	/*m_displayPoints.clear();

	for (int i = 0; i < m_virtualPoints.size(); i++)
	{
		std::weak_ptr<Node> weaknode = m_virtualPoints[i];
		m_displayPoints.push_back(weaknode);
	}

	if (auto parent = m_parent.lock())
	{
		GroupNode* gParent = dynamic_cast<GroupNode*>(parent.get());
		gParent->SetChildren(m_displayPoints);
	}*/
#endif

	m_meshDesc.vertices = vertices;
	m_meshDesc.indices = indices;
	m_meshDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
}

void InterpolationBezierCurveC2::PreparePolygonDesc()
{	
	std::vector<VertexPositionColor> interpolationCurveVertices;
	std::vector<unsigned short> interpolationCurveIndices;
	for (int i = 0; i < m_controlPoints.size(); i++)
	{
		if (auto point = m_controlPoints[i].lock())
		{
			interpolationCurveVertices.push_back(VertexPositionColor{
				point->m_object->GetPosition(),
				m_PolygonDesc.m_defaultColor
				}
			);
			interpolationCurveIndices.push_back(i);
		}
	}

	m_PolygonDesc.vertices = interpolationCurveVertices;
	m_PolygonDesc.indices = interpolationCurveIndices;
	m_PolygonDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
}
