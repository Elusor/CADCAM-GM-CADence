#include "bezierCalculator.h"
#include "BezierCurveC2.h"
#include "mathUtils.h"
#include "Node.h"
#include "adaptiveRenderingCalculator.h"
#include "GroupNode.h"
#include "imgui.h"

BezierCurveC2::BezierCurveC2(): BezierCurveC2(std::vector<std::weak_ptr<Node>>(), BezierBasis::BSpline)
{
}

BezierCurveC2::BezierCurveC2(std::vector<std::weak_ptr<Node>> initialControlPoints, BezierBasis basis)
{
	m_adaptiveRenderingSamples = 0;
	m_renderPolygon = false;
	m_renderDeBoorPolygon = false;	
}

void BezierCurveC2::Initialize(std::vector<std::weak_ptr<Node>> initialControlPoints, BezierBasis basis)
{
	assert(!m_nodePtr.expired());

	m_renderPolygon = basis == BezierBasis::Bernstein;
	m_renderDeBoorPolygon = basis == BezierBasis::BSpline;

	for (int i = 0; i < initialControlPoints.size(); i++)
	{
		GetReferences().LinkRef(initialControlPoints[i]);
	}

	m_basis = basis;
	RecalculateBasisPoints();
}

void BezierCurveC2::UpdateObject()
{
	RecalculateIfModified();
	if (GetReferences().GetAllRef().size() >= 4)
	{

		m_virtualBernsteinPos = CalculateBernsteinFromDeBoor();
		UpdateGSData();
		PreparePolygonDesc();
	}
}

void BezierCurveC2::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	RemoveExpiredChildren();

	if (GetReferences().GetAllRef().size() >= 4)
	{		
		RenderCurve(renderState);
		RenderPolygon(renderState);
	}
}

void BezierCurveC2::AttachChild(std::weak_ptr<Node> controlPoint)
{
	GetReferences().LinkRef(controlPoint);
	RecalculateBasisPoints();
	SetModified(true);
}

void BezierCurveC2::RemoveChild(std::weak_ptr<Node> controlPoint)
{
	auto controlPointRefs = GetReferences().GetAllRef();
	GetReferences().UnlinkRef(controlPoint);	
	RecalculateBasisPoints(true);
	SetModified(true);
}

bool BezierCurveC2::CreateParamsGui()
{
	bool objectChanged = BezierCurve::CreateParamsGui();
	ImGui::Begin("Inspector");	

	std::string label = "Display De Boor polygon" + GetIdentifier();
	objectChanged |= ImGui::Checkbox(label.c_str(), &m_renderDeBoorPolygon);
	ImGui::Spacing();

	// change colors for polygon
	float pcolor[3] = {
		m_deBoorPolyDesc.m_defaultColor.x,
		m_deBoorPolyDesc.m_defaultColor.y,
		m_deBoorPolyDesc.m_defaultColor.z,
	};

	std::string ptext = "De Boor Polygon color";
	ImGui::Text(ptext.c_str());
	objectChanged |= ImGui::ColorEdit3(("##" + ptext + GetIdentifier()).c_str(), (float*)&pcolor);

	m_deBoorPolyDesc.m_defaultColor.x = pcolor[0];
	m_deBoorPolyDesc.m_defaultColor.y = pcolor[1];
	m_deBoorPolyDesc.m_defaultColor.z = pcolor[2];
	ImGui::Spacing();

	bool clicked = ImGui::Button("Change basis");

	objectChanged |= clicked;

	if (clicked)
	{
		SwitchBases();
	}
	ImGui::End();
	return objectChanged;
}

void BezierCurveC2::RecalculateBasisPoints(bool overwriteVertices)
{
	if(overwriteVertices)
		m_curBasisControlPoints.clear();

	if (m_basis == BezierBasis::Bernstein)
	{
		RecalculateBernsteinPoints(overwriteVertices);
	}

	if (m_basis == BezierBasis::BSpline)
	{
		RecalculateBSplinePoints(overwriteVertices);
	}
}

std::vector<std::weak_ptr<Node>> BezierCurveC2::GetDisplayChildren()
{
	return m_curBasisControlPoints;
}

bool BezierCurveC2::GetIsModified()
{
	auto controlPointRefs = GetReferences().GetAllRef();

	if (m_basis == BezierBasis::Bernstein)
	{
		for (int i = 0; i < m_curBasisControlPoints.size(); i++)
		{
			if (auto curCtrlPoint = m_curBasisControlPoints[i].lock())
			{
				if (curCtrlPoint->m_object->GetIsModified())
				{
					SetModified(true);
				}
			}			
		}
	}

	for (int i = 0; i < controlPointRefs.size(); i++)
	{
		if (auto point = controlPointRefs[i].m_refered.lock())
		{			
			if (point->m_object->GetIsModified())
			{
				SetModified(true);
			}
			else {

			}
		}
	}

	if (RemoveExpiredChildren())
		SetModified(true);

	return m_modified;
}

void BezierCurveC2::SetDisplayDeBoorPolygon(bool isDisplayed)
{
	m_renderDeBoorPolygon = isDisplayed;
}

bool BezierCurveC2::GetDisplayDeBoorPolygon()
{
	return m_renderDeBoorPolygon;
}

BezierBasis BezierCurveC2::GetCurrentBasis()
{
	return m_basis;
}

bool BezierCurveC2::RemoveExpiredChildren()
{
	bool removed = GetReferences().RemovedExpiredReferences();

	if (auto parent = m_nodePtr.lock())
	{
		auto gParent = dynamic_cast<GroupNode*>(parent.get());
		gParent->RemoveExpiredChildren();
	}

	auto it2 = m_curBasisControlPoints.begin();
	while (it2 != m_curBasisControlPoints.end())
	{
		if (it2->expired())
		{
			it2 = m_curBasisControlPoints.erase(it2);
			removed = true;
		}
		else {
			it2++;
		}
	}

	if(removed)
		RecalculateBasisPoints();

	return removed;
}

void BezierCurveC2::RecalculateBSplinePoints(bool overwriteVertices)
{
	auto controlPoints = GetControlPoints();

	if (overwriteVertices)
	{
		for (int i = 0; i < controlPoints.size(); i++)
		{
			m_curBasisControlPoints.push_back(controlPoints[i]);
		}

		if (auto parent = m_nodePtr.lock())
		{
			GroupNode* gParent = dynamic_cast<GroupNode*>(parent.get());
			gParent->SetChildren(m_curBasisControlPoints);
		}
	}
	else {

		for (int i = 0; i < controlPoints.size(); i++)
		{
			if (auto point = controlPoints[i].lock())
			{
				if (auto ctrlPoint = m_curBasisControlPoints[i].lock())
				{
					ctrlPoint->m_object->SetPosition(point->m_object->GetPosition());
					ctrlPoint->m_object->SetModified(true);
				}				
			}			
		}
	}		
}

void BezierCurveC2::RecalculateBernsteinPoints(bool overwriteVertices)
{
	m_virtualBernsteinPos = CalculateBernsteinFromDeBoor();
	
	if (overwriteVertices)
	{
		for (int i = 0; i < m_virtualBernsteinPos.size(); i++)
		{
			Point* p = new Point();
			p->m_defaultName = p->m_name = "Bernstein point " + std::to_string(i);
			p->SetPosition(m_virtualBernsteinPos[i]);

			Node* node = new Node();
			node->SetIsVirtual(true);
			node->m_object = std::unique_ptr<Point>(p);
			// fill m_curBasisControlPoints with Bernstein points - that is points calculated from m_virtualBernsteinPoints	
			std::shared_ptr<Node> nodeptr = std::shared_ptr<Node>(node);
			m_virtualBernsteinPoints.push_back(nodeptr);
			std::weak_ptr<Node> weaknode = nodeptr;
			m_curBasisControlPoints.push_back(weaknode);
		}

		if (auto parent = m_nodePtr.lock())
		{
				GroupNode* gParent = dynamic_cast<GroupNode*>(parent.get());
				gParent->SetChildren(m_curBasisControlPoints);
		}
	}	
	else
	{
		if (auto parent = m_nodePtr.lock())
		{
			GroupNode* gParent = dynamic_cast<GroupNode*>(parent.get());
			auto children = gParent->GetChildren();
			for (int i = 0; i < children.size(); i++)
			{
				auto child = children[i];
				if (auto childLock = child.lock())
				{
					childLock->m_object->SetPosition(m_virtualBernsteinPos[i]);
				}
			}
		}
	}
}

void BezierCurveC2::SwitchBases()
{
	// Switch TO BSPLINE
	if (m_basis == BezierBasis::Bernstein)
	{		
		m_basis = BezierBasis::BSpline;
	}
	else {
		// Switch TO BERNSTEIN
		if (m_basis == BezierBasis::BSpline)
		{
			m_basis = BezierBasis::Bernstein;
		}
	}
	RecalculateBasisPoints();
}

std::vector<DirectX::XMFLOAT3> BezierCurveC2::CalculateBernsteinFromDeBoor()
{
	std::vector<DirectX::XMFLOAT3> deBoorPoints;
	std::vector<DirectX::XMFLOAT3> innerControlPoints;
	std::vector<DirectX::XMFLOAT3> bernsteinPoints;

	auto controlPoints = GetControlPoints();

	for (int i = 0; i < controlPoints.size(); i++)
	{
		if (auto point = controlPoints[i].lock())
		{
			deBoorPoints.push_back(
				point->m_object->GetPosition()
			);
		}
	}

	if (controlPoints.size() >= 4)
	{		
		innerControlPoints.push_back(
			F3Lerp(
				deBoorPoints[0],
				deBoorPoints[1],
				1.0f / 3.0f));

		// calculate inner control points between deBoor points
		for (int i = 1; i < deBoorPoints.size() - 2; i++)
		{
			auto pos1 = F3Lerp(
				deBoorPoints[i],
				deBoorPoints[i + 1],
				2.0f / 3.0f);

			auto pos2 = F3Lerp(
				deBoorPoints[i],
				deBoorPoints[i + 1],
				1.0f / 3.0f);

			innerControlPoints.push_back(pos1);
			innerControlPoints.push_back(pos2);
		}

		innerControlPoints.push_back(
			F3Lerp(
				deBoorPoints[deBoorPoints.size() - 2],
				deBoorPoints[deBoorPoints.size() - 1],
				2.0f / 3.0f));

		for (int i = 0; i < innerControlPoints.size(); i += 2)
		{
			// insert first point 
			bernsteinPoints.push_back(innerControlPoints[i]);
			// get 1/2 point between i and i+1
			bernsteinPoints.push_back(
				F3Lerp(
					innerControlPoints[i],
					innerControlPoints[i + 1],
				1.0f / 2.0f));
			// insert second point
			bernsteinPoints.push_back(innerControlPoints[i+1]);
		}
			// remove excess points
		bernsteinPoints.erase(bernsteinPoints.begin()+0);
		bernsteinPoints.erase(bernsteinPoints.end()-1);				
	}

	return bernsteinPoints;
}

void BezierCurveC2::MoveBernsteinPoint(int index)
{
	auto controlPoints = GetControlPoints();
	if (auto movedPt = m_curBasisControlPoints[index].lock())
	{
		DirectX::XMFLOAT3 movedPtPos = movedPt->m_object->GetPosition();

		int controlPointIndex = (index + 1) / 3 + 1;

		bool isMiddle = index % 3 == 0; // selected bernstein point is under the current De boor point
		bool isAfter = index % 3 == 1; // selected bernstein point is placed on the line between curent and next de boor points

		bool allValid = true;

		std::shared_ptr<Node> prevDB = controlPoints[controlPointIndex - 1].lock();
		std::shared_ptr<Node> curDB = controlPoints[controlPointIndex].lock();
		std::shared_ptr<Node> nextDB = controlPoints[controlPointIndex + 1].lock();

		DirectX::XMFLOAT3 prevDBPos = prevDB->m_object->GetPosition();
		DirectX::XMFLOAT3 curDBPos = curDB->m_object->GetPosition();
		DirectX::XMFLOAT3 nextDBPos = nextDB->m_object->GetPosition();

		DirectX::XMFLOAT3 newDBcoords = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

		if (isMiddle)
		{ // bernstein point is under the de boor point
			DirectX::XMFLOAT3 transToLine = XMF3TimesFloat(XMF3SUB(nextDBPos, prevDBPos), 1.0f / 6.0f);
			DirectX::XMFLOAT3 posOnLine = XMF3SUM(movedPtPos, transToLine); // position of moved de boor and 1/6 of the diff between DBnext and DB[rev
			DirectX::XMFLOAT3 diff = XMF3SUB(posOnLine, nextDBPos); // dif between pos on line and next De Boor
			newDBcoords = XMF3SUM(nextDBPos, XMF3TimesFloat(diff, 3.0f / 2.0f));
		}
		else
		{
			if (isAfter)
			{ // bernstein point is after the de boor point
				DirectX::XMFLOAT3 diff = XMF3SUB(movedPtPos, nextDBPos); // dif between pos on line and next De Boor
				newDBcoords = XMF3SUM(nextDBPos, XMF3TimesFloat(diff, 3.0f / 2.0f));
			}
			else
			{ // bernstein point is before the de boor point
				DirectX::XMFLOAT3 diff = XMF3SUB(movedPtPos, prevDBPos); // dif between pos on line and next De Boor
				newDBcoords = XMF3SUM(prevDBPos, XMF3TimesFloat(diff, 3.0f / 2.0f));
			}
		}

		curDB->m_object->SetPosition(newDBcoords);
		curDB->m_object->SetModified(true);
	}
	
}

void BezierCurveC2::UpdateGSData()
{
	m_lastVertexDuplicationCount = 0;
	std::vector<VertexPositionColor> vertices;
	std::vector<unsigned short> indices;

	for (int i = 0; i < m_virtualBernsteinPos.size(); i++)
	{
		//add all vertices
		auto pos = m_virtualBernsteinPos[i];
		vertices.push_back(VertexPositionColor{
					pos,
					m_meshDesc.m_defaultColor });
	}

	for (int i = 0; i < m_virtualBernsteinPos.size(); i += 3)
	{
		// mage edges out of the vertices
		// There are some elements that should be added
		if (m_virtualBernsteinPos.size() - i > 1)
		{
			// add next 4 points normally
			if (m_virtualBernsteinPos.size() - i >= 4)
			{
				indices.push_back(i);
				indices.push_back(i + 1);
				indices.push_back(i + 2);
				indices.push_back(i + 3);
			}
			// add the rest of the nodes and add the last node multiple times
			else {
				for (int j = i; j < m_virtualBernsteinPos.size(); j++)
				{
					indices.push_back(j);
				}

				// add the rest of the vertices as duplicates of the last one
				int emptyVertices = 4 - (m_virtualBernsteinPos.size() - i);
				//m_lastVertexDuplicationCount = emptyVertices;
				for (int k = 0; k < emptyVertices; k++)
				{
					indices.push_back(m_virtualBernsteinPos.size() - 1);
				}
			}

		}
	}

	m_meshDesc.vertices = vertices;
	m_meshDesc.indices = indices;
	m_meshDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
}

void BezierCurveC2::PreparePolygonDesc()
{
	auto controlPoints = GetControlPoints();

	std::vector<VertexPositionColor> bernCurveVertices;
	std::vector<unsigned short> bernCurveIndices;
	for (int i = 0; i < m_virtualBernsteinPos.size(); i++)
	{
		bernCurveVertices.push_back(VertexPositionColor{
			m_virtualBernsteinPos[i],
			m_PolygonDesc.m_defaultColor
			}
		);
		bernCurveIndices.push_back(i);
	}
	m_PolygonDesc.vertices = bernCurveVertices;
	m_PolygonDesc.indices = bernCurveIndices;
	m_PolygonDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;



	std::vector<VertexPositionColor> deBoorCurveVertices;
	std::vector<unsigned short> deBoorCurveIndices;
	for (int i = 0; i < controlPoints.size(); i++)
	{
		if (auto point = controlPoints[i].lock())
		{
			deBoorCurveVertices.push_back(VertexPositionColor{
				point->m_object->GetPosition(),
				m_deBoorPolyDesc.m_defaultColor
				}
			);
			deBoorCurveIndices.push_back(i);
		}
	}

	m_deBoorPolyDesc.vertices = deBoorCurveVertices;
	m_deBoorPolyDesc.indices = deBoorCurveIndices;
	m_deBoorPolyDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
}

void BezierCurveC2::RecalculateIfModified()
{
	auto controlPoints = GetControlPoints();

	if (RemoveExpiredChildren())
	{
		RecalculateBasisPoints();
	}

	// check if any virtual Bernstein nodes have been modified and recalculate proper deBoor points	
	int modifiedIndex = -1;
	if (m_basis == BezierBasis::Bernstein)
	{
		for (int i = 0; i < m_curBasisControlPoints.size(); i++)
		{
			if (m_curBasisControlPoints[i].lock()->m_object->GetIsModified())
			{
				modifiedIndex = i;
			}
		}
		if (modifiedIndex >= 0)
		{
			MoveBernsteinPoint(modifiedIndex);
			RecalculateBasisPoints(false);
		}
	}

	modifiedIndex = -1;
	for (int i = 0; i < controlPoints.size(); i++)
	{
		if (auto point = controlPoints[i].lock())
		{
			if (point->m_object->GetIsModified())
			{
				modifiedIndex = i;
			}
		}
	}
	if (modifiedIndex >= 0)
	{
		//MoveBernsteinPoint(modifiedIndex);
		RecalculateBasisPoints(false);
	}
}

void BezierCurveC2::RenderCurve(std::unique_ptr<RenderState>& renderState)
{
	auto controlPoints = GetControlPoints();

	// This should actually be in update (late update?)
	CalculateAdaptiveRendering(controlPoints, renderState);

	if (m_modified)
	{
		UpdateObject();
	}

	// Set up GS buffer
	DirectX::XMFLOAT4 data = DirectX::XMFLOAT4(m_adaptiveRenderingSamples / 20, m_lastVertexDuplicationCount, 0.0f, 0.0f);
	DirectX::XMVECTOR GSdata = DirectX::XMLoadFloat4(&data);
	auto buf = renderState->SetConstantBuffer<DirectX::XMVECTOR>(renderState->m_cbGSData.get(), GSdata);
	ID3D11Buffer* cbs[] = { buf };
	renderState->m_device.context()->GSSetConstantBuffers(0, 1, cbs);

	// Turn on bezier geometry shader
	renderState->m_device.context()->GSSetShader(renderState->m_bezierGeometryShader.get(), nullptr, 0);
	MeshObject::RenderObject(renderState);
	// turn off bezier geometry shader
	renderState->m_device.context()->GSSetShader(nullptr, nullptr, 0);

}

void BezierCurveC2::RenderPolygon(std::unique_ptr<RenderState>& renderState)
{
	if (m_renderPolygon)
	{
		RenderMesh(renderState, m_PolygonDesc);
	}

	if (m_renderDeBoorPolygon)
	{
		RenderMesh(renderState, m_deBoorPolyDesc);
	}
}
