#include "bezierCalculator.h"
#include "BezierCurveC2.h"
#include "mathUtils.h"
#include "Node.h"


BezierCurveC2::BezierCurveC2(): BezierCurveC2(std::vector<std::weak_ptr<Node>>(), BezierBasis::BSpline)
{
}

BezierCurveC2::BezierCurveC2(std::vector<std::weak_ptr<Node>> initialControlPoints, BezierBasis basis)
{
	m_renderPolygon = false;
	m_controlPoints = initialControlPoints;
	m_basis = BezierBasis::BSpline;
}

void BezierCurveC2::SwitchBases(BezierBasis basis)
{


}

void BezierCurveC2::UpdateObject()
{
	if (m_controlPoints.size() >= 4)
	{
		m_virtualBernsteinPoints = CalculateBezierFromDeBoor();
		std::vector<VertexPositionColor> curveVertices;
		std::vector<unsigned short> curveIndices;
		auto curvePoints = BezierCalculator::CalculateBezierDeCasteljau(m_virtualBernsteinPoints, 200);		
		for (int i = 0; i < curvePoints.size(); i++)
		{
			curveVertices.push_back(VertexPositionColor{
				curvePoints[i],
				m_meshDesc.m_defaultColor
				});
			curveIndices.push_back(i);
		}
		m_meshDesc.vertices = curveVertices;
		m_meshDesc.indices = curveIndices;
		m_meshDesc.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;


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
	}
}

void BezierCurveC2::RenderObject(std::unique_ptr<RenderState>& renderState)
{
	if (m_controlPoints.size() >= 4)
	{
		/*renderData->m_device.context()->GSSetShader(
			renderData->m_bezierGeometryShader.get(),
			nullptr, 0);*/

		UpdateObject();
		MeshObject::RenderObject(renderState);

		//renderData->m_device.context()->GSSetShader(nullptr, nullptr, 0);
		if (m_renderPolygon)
		{
			RenderPolygon(renderState);
		}
	}
}

std::vector<DirectX::XMFLOAT3> BezierCurveC2::CalculateBezierFromDeBoor()
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
		// get 2/3 point between 0 an 1 
		auto p3 = 0;//weightedSum(p0,p1,1/3)

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
