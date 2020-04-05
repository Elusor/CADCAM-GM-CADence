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
	m_renderPolygon = basis == BezierBasis::Bernstein;
	m_renderDeBoorPolygon = basis == BezierBasis::BSpline;
	m_controlPoints = initialControlPoints;
	m_basis = basis;
	RecalculateBasisPoints();
}

void BezierCurveC2::UpdateObject()
{
	// check if any virtual Bernstein nodes have been modified and recalculate proper deBoor points	
	int modifiedIndex = -1;
	if (m_basis == BezierBasis::Bernstein)
	{
		for (int i = 0; i < m_curBasisControlPoints.size(); i++)
		{
			if (m_curBasisControlPoints[i]->m_object->GetIsModified())
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
	for (int i = 0; i < m_controlPoints.size(); i++)
	{
		if (auto point = m_controlPoints[i].lock())
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

	if (m_controlPoints.size() >= 4)
	{

		m_virtualBernsteinPoints = CalculateBernsteinFromDeBoor();
#pragma region gs format
		std::vector<VertexPositionColor> vertices;
		std::vector<unsigned short> indices;

		for (int i = 0; i < m_virtualBernsteinPoints.size(); i++)
		{
			//add all vertices
			auto pos = m_virtualBernsteinPoints[i];
			vertices.push_back(VertexPositionColor{
						pos,
						m_meshDesc.m_defaultColor });
		}

		for (int i = 0; i < m_virtualBernsteinPoints.size(); i += 3)
		{
			// mage edges out of the vertices
			// There are some elements that should be added
			if (m_virtualBernsteinPoints.size() - i > 1);
			{
				// add next 4 points normally
				if (m_virtualBernsteinPoints.size() - i >= 4)
				{
					indices.push_back(i);
					indices.push_back(i + 1);
					indices.push_back(i + 2);
					indices.push_back(i + 3);
				}
				// add the rest of the nodes and add the last node multiple times
				else {
					for (int j = i; j < m_virtualBernsteinPoints.size(); j++)
					{
						indices.push_back(j);
					}

					// add the rest of the vertices as duplicates of the last one
					int emptyVertices = 4 - (m_virtualBernsteinPoints.size() - i);
					for (int k = 0; k < emptyVertices; k++)
					{
						indices.push_back(m_virtualBernsteinPoints.size() - 1);
					}
				}

			}
		}

		m_meshDesc.vertices = vertices;
		m_meshDesc.indices = indices;
		m_meshDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
#pragma endregion

		std::vector<VertexPositionColor> bernCurveVertices;
		std::vector<unsigned short> bernCurveIndices;
		for (int i = 0; i < m_virtualBernsteinPoints.size(); i++)
		{
			bernCurveVertices.push_back(VertexPositionColor{
				m_virtualBernsteinPoints[i],
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
		for (int i = 0; i < m_controlPoints.size(); i++)
		{
			if (auto point = m_controlPoints[i].lock())
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
}

void BezierCurveC2::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	RemoveExpiredChildren();
	if (m_controlPoints.size() >= 4)
	{
		

		// This should actually be in update
		int prev = m_adaptiveRenderingSamples;
		m_adaptiveRenderingSamples = AdaptiveRenderingCalculator::CalculateAdaptiveSamplesCount(m_controlPoints, renderState);
		if (prev != m_adaptiveRenderingSamples)
			SetModified(true);

		if (m_modified)
		{
			UpdateObject();
		}

		// Turn on bezier geometry shader
		renderState->m_device.context()->GSSetShader(
			renderState->m_bezierGeometryShader.get(),
			nullptr, 0);
		MeshObject::RenderObject(renderState);
		// turn off bezier geometry shader
		renderState->m_device.context()->GSSetShader(nullptr, nullptr, 0);

		//renderData->m_device.context()->GSSetShader(nullptr, nullptr, 0);
		if (m_renderPolygon)
		{
			RenderMesh(renderState, m_PolygonDesc);
		}

		if (m_renderDeBoorPolygon)
		{
			RenderMesh(renderState, m_deBoorPolyDesc);
		}
	}
}

void BezierCurveC2::AttachChild(std::weak_ptr<Node> controlPoint)
{
	m_controlPoints.push_back(controlPoint);
	RecalculateBasisPoints();
	SetModified(true);
}

void BezierCurveC2::RemoveChild(std::weak_ptr<Node> controlPoint)
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
	// This needs to be called with false not to overwrite the vector on which the calling function in iterating
	// Rewrite both of these methods to make them implementation - agnostic
	RecalculateBasisPoints(false);
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

bool BezierCurveC2::GetIsModified()
{
	if (m_basis == BezierBasis::Bernstein)
	{
		for (int i = 0; i < m_curBasisControlPoints.size(); i++)
		{
			if (m_curBasisControlPoints[i]->m_object->GetIsModified())
			{
				SetModified(true);
			}
		}
	}

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

void BezierCurveC2::RecalculateBSplinePoints(bool overwriteVertices)
{
	for (int i = 0; i < m_controlPoints.size(); i++)
	{
		if (auto point = m_controlPoints[i].lock())
		{
			m_curBasisControlPoints.push_back(point);
		}
	}

	if (auto parent = m_parent.lock())
	{
		if (overwriteVertices)
		{
			GroupNode* gParent = dynamic_cast<GroupNode*>(parent.get());
			gParent->SetChildren(m_curBasisControlPoints);
		}
	}
}

void BezierCurveC2::RecalculateBernsteinPoints(bool overwriteVertices)
{
	m_virtualBernsteinPoints = CalculateBernsteinFromDeBoor();
	
	if (overwriteVertices)
	{
		for (int i = 0; i < m_virtualBernsteinPoints.size(); i++)
		{
			Point* p = new Point();
			p->m_defaultName = p->m_name = "Bernstein point " + std::to_string(i);
			p->SetPosition(m_virtualBernsteinPoints[i]);

			Node* node = new Node();
			node->m_isVirtual = true;
			node->m_object = std::unique_ptr<Point>(p);
			// fill m_curBasisControlPoints with Bernstein points - that is points calculated from m_virtualBernsteinPoints
			m_curBasisControlPoints.push_back(std::shared_ptr<Node>(node));
		}

		if (auto parent = m_parent.lock())
		{
				GroupNode* gParent = dynamic_cast<GroupNode*>(parent.get());
				gParent->SetChildren(m_curBasisControlPoints);			
		}
	}	
	else
	{
		if (auto parent = m_parent.lock())
		{
			GroupNode* gParent = dynamic_cast<GroupNode*>(parent.get());
			auto children = gParent->GetChildren();
			for (int i = 0; i < children.size(); i++)
			{
				auto child = children[i];
				if (auto childLock = child.lock())
				{
					childLock->m_object->SetPosition(m_virtualBernsteinPoints[i]);
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

	for (int i = 0; i < m_controlPoints.size(); i++)
	{
		if (auto point = m_controlPoints[i].lock())
		{
			deBoorPoints.push_back(
				point->m_object->GetPosition()
			);
		}
	}
	
	if (m_controlPoints.size() >= 4)
	{		
		innerControlPoints.push_back(
			WeightedXMFloat3Average(
				deBoorPoints[0],
				deBoorPoints[1],
				1.0f / 3.0f));

		// calculate inner control points between deBoor points
		for (int i = 1; i < deBoorPoints.size() - 2; i++)
		{
			auto pos1 = WeightedXMFloat3Average(
				deBoorPoints[i],
				deBoorPoints[i + 1],
				2.0f / 3.0f);

			auto pos2 = WeightedXMFloat3Average(
				deBoorPoints[i],
				deBoorPoints[i + 1],
				1.0f / 3.0f);

			innerControlPoints.push_back(pos1);
			innerControlPoints.push_back(pos2);
		}

		innerControlPoints.push_back(
			WeightedXMFloat3Average(
				deBoorPoints[deBoorPoints.size() - 2],
				deBoorPoints[deBoorPoints.size() - 1],
				2.0f / 3.0f));

		for (int i = 0; i < innerControlPoints.size(); i += 2)
		{
			// insert first point 
			bernsteinPoints.push_back(innerControlPoints[i]);
			// get 1/2 point between i and i+1
			bernsteinPoints.push_back(
				WeightedXMFloat3Average(
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
	DirectX::XMFLOAT3 movedPoint = m_curBasisControlPoints[index]->m_object->GetPosition();

	int controlPointIndex = (index + 1) / 3 + 1;
	
	bool isMiddle = index % 3 == 0; // selected bernstein point is under the current De boor point
	bool isAfter = index % 3 == 1; // selected bernstein point is placed on the line between curent and next de boor points

	bool allValid = true;

	std::shared_ptr<Node> prevDB = m_controlPoints[controlPointIndex - 1].lock();
	std::shared_ptr<Node> curDB =  m_controlPoints[controlPointIndex].lock();
	std::shared_ptr<Node> nextDB = m_controlPoints[controlPointIndex + 1].lock();

	DirectX::XMFLOAT3 prevDBPos = prevDB->m_object->GetPosition();
	DirectX::XMFLOAT3 curDBPos = curDB->m_object->GetPosition();
	DirectX::XMFLOAT3 nextDBPos = nextDB->m_object->GetPosition();

	DirectX::XMFLOAT3 newDBcoords = DirectX::XMFLOAT3(0.0f,0.0f,0.0f);

	if (isMiddle)
	{ // bernstein point is under the de boor point
		DirectX::XMFLOAT3 transToLine = XMFloat3TimesFloat(XMF3SUB(nextDBPos, prevDBPos), 1.0f/6.0f);
		DirectX::XMFLOAT3 posOnLine = XMF3SUM(movedPoint, transToLine); // position of moved de boor and 1/6 of the diff between DBnext and DB[rev
		DirectX::XMFLOAT3 diff = XMF3SUB(posOnLine, nextDBPos); // dif between pos on line and next De Boor
		newDBcoords = XMF3SUM(nextDBPos, XMFloat3TimesFloat(diff, 3.0f / 2.0f));		
	}
	else 
	{
		if (isAfter)
		{ // bernstein point is after the de boor point
			DirectX::XMFLOAT3 diff = XMF3SUB(movedPoint, nextDBPos); // dif between pos on line and next De Boor
			newDBcoords = XMF3SUM(nextDBPos, XMFloat3TimesFloat(diff, 3.0f / 2.0f));
		}
		else 
		{ // bernstein point is before the de boor point
			DirectX::XMFLOAT3 diff = XMF3SUB(movedPoint, prevDBPos); // dif between pos on line and next De Boor
			newDBcoords = XMF3SUM(prevDBPos, XMFloat3TimesFloat(diff, 3.0f / 2.0f));
		}
	}

	curDB->m_object->SetPosition(newDBcoords);

}