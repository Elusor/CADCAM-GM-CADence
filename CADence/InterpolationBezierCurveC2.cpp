#include "InterpolationBezierCurveC2.h"
#include "GroupNode.h"
DirectX::XMMATRIX changeBasisMtx = {
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
	bool objectChanged = BezierCurve::CreateParamsGui();
	ImGui::Begin("Inspector");

	std::string label = "Display polygon" + GetIdentifier();
	objectChanged |= ImGui::Checkbox(label.c_str(), &m_renderPolygon);
	ImGui::Spacing();

	// change colors for polygon
	float pcolor[3] = {
		m_PolygonDesc.m_defaultColor.x,
		m_PolygonDesc.m_defaultColor.y,
		m_PolygonDesc.m_defaultColor.z,
	};

	std::string ptext = "De color";
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

	std::vector<DirectX::XMFLOAT3> a, b, c, d;

	diag.push_back(2.0f);
	for (int i = 0; i < interpolationKnots.size() - 2; i++)
	{
		diag.push_back(4.0f);
	}
	diag.push_back(2.0f);

	lowerDiag.push_back(0.f);
	for (int i = 0; i < interpolationKnots.size() - 1; i++)
	{
		upperDiag.push_back(1.0f);
		lowerDiag.push_back(1.0f);
	}
	upperDiag.push_back(0.f);
	// insert the first point
	auto p0 = interpolationKnots[0].lock();
	auto p1 = interpolationKnots[1].lock();
	auto pnPos = p0->m_object->GetPosition();
	auto pnPosPrev = p1->m_object->GetPosition();
	auto diff = XMF3SUB(pnPos, pnPosPrev);
	xVector.push_back(3 * diff.x);
	yVector.push_back(3 * diff.y);
	zVector.push_back(3 * diff.z);

	for (int i = 0; i < interpolationKnots.size() - 2; i++)
	{
		//insert 3(point[i+2] - point[i])
		auto pi = interpolationKnots[i].lock();
		auto pinext = interpolationKnots[i + 2].lock();
		auto pnPos = pi->m_object->GetPosition();
		auto pnPosNext = pinext->m_object->GetPosition();
		diff = XMF3SUB(pnPosNext, pnPos);
		xVector.push_back(3 * diff.x);
		yVector.push_back(3 * diff.y);
		zVector.push_back(3 * diff.z);

	}
	// insert the last point
	auto pnprev = interpolationKnots[interpolationKnots.size() - 2].lock();
	auto pn = interpolationKnots[interpolationKnots.size() - 1].lock();
	pnPos = pn->m_object->GetPosition();
	pnPosPrev = pnprev->m_object->GetPosition();
	diff = XMF3SUB(pnPos, pnPosPrev);
	xVector.push_back(3 * diff.x);
	yVector.push_back(3 * diff.y);
	zVector.push_back(3 * diff.z);


	auto xRes = SolveTridiagMatrix(lowerDiag, diag, lowerDiag, xVector);
	auto yRes = SolveTridiagMatrix(lowerDiag, diag, lowerDiag, yVector);
	auto zRes = SolveTridiagMatrix(lowerDiag, diag, lowerDiag, zVector);

	//calc a b c d for each res
	for (int i = 0; i < interpolationKnots.size() - 1; i++)
	{
		auto pti = interpolationKnots[i].lock();
		auto pti1 = interpolationKnots[i + 1].lock();
		auto ptiPos = pti->m_object->GetPosition();
		//diff = (yi+1 - yi)		
		auto diff = XMF3SUB(pti1->m_object->GetPosition(), pti->m_object->GetPosition());
		auto Di = DirectX::XMFLOAT3(xRes[i], yRes[i], zRes[i]);
		auto Di1 = DirectX::XMFLOAT3(xRes[i + 1], yRes[i + 1], zRes[i + 1]);
		// a = y
		a.push_back(ptiPos);
		// b = Di
		b.push_back(Di);
		// c = 3 diff - 2Di - Di+1
		c.push_back(XMFloat3TimesFloat(diff, 3));
		c[i] = XMF3SUB(c[i], XMFloat3TimesFloat(Di, 2));
		c[i] = XMF3SUB(c[i], Di1);
		// d = -2 diff + Di + Di+1
		d.push_back(XMFloat3TimesFloat(diff, -2));
		d[i] = XMF3SUM(c[i], Di);
		d[i] = XMF3SUM(c[i], Di1);
	}

	std::vector<DirectX::XMFLOAT3> resultPos;

	// convert abcd to bernstein basis
	for (int i = 0; i < a.size(); i++)
	{
		DirectX::XMFLOAT4X4 resMat;
		DirectX::XMMATRIX vectorMat = {
			a[i].x, b[i].x, c[i].x, d[i].x,
			a[i].y, b[i].y, c[i].y, d[i].y,
			a[i].z, b[i].z, c[i].z, d[i].z,
			0.f, 0.f, 0.f, 0.f
		};
	
		auto res = vectorMat * changeBasisMtx;

		DirectX::XMStoreFloat4x4(&resMat, res);

		a[i] = DirectX::XMFLOAT3(resMat._11, resMat._21, resMat._31);
		b[i] = DirectX::XMFLOAT3(resMat._12, resMat._22, resMat._32);
		c[i] = DirectX::XMFLOAT3(resMat._13, resMat._23, resMat._33);
		d[i] = DirectX::XMFLOAT3(resMat._14, resMat._24, resMat._34);

		resultPos.push_back(a[i]);
		resultPos.push_back(b[i]);
		resultPos.push_back(c[i]);
		if (i == xRes.size() - 1)
		{
			resultPos.push_back(d[i]);
		}
	}

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
		if (m_virtualPoints.size() - i > 1);
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

void InterpolationBezierCurveC2::PreparePolygonDesc()
{	
	std::vector<VertexPositionColor> interpolationCurveVertices;
	std::vector<unsigned short> interpolationCurveIndices;
	for (int i = 0; i < m_virtualPoints.size(); i++)
	{
		if (auto point = m_virtualPoints[i])
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
