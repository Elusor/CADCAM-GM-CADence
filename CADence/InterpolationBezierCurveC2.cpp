#include "InterpolationBezierCurveC2.h"
DirectX::XMMATRIX changeBasisMtx = {
		1.f, 1.f, 1.f, 1.f,
		0.f,  1.f / 3.f, 2.f / 3.f, 1.f,
		0.f, 0.f, 1.f / 3.f, 1.f,
		0.f, 0.f, 0.f, 1.f };

InterpolationBezierCurveC2::InterpolationBezierCurveC2()
{
	m_adaptiveRenderingSamples = 0;
}

InterpolationBezierCurveC2::InterpolationBezierCurveC2(std::vector<std::weak_ptr<Node>> initialKnots)
{
}

void InterpolationBezierCurveC2::UpdateObject()
{
	RecalculateIfModified();


}

bool InterpolationBezierCurveC2::CreateParamsGui()
{
	return false;
}

void InterpolationBezierCurveC2::RenderObject(std::unique_ptr<RenderState>& renderState)
{
}

void InterpolationBezierCurveC2::AttachChild(std::weak_ptr<Node> controlPoint)
{
}

void InterpolationBezierCurveC2::RemoveChild(std::weak_ptr<Node> controlPoint)
{
}

bool InterpolationBezierCurveC2::GetIsModified()
{
	return false;
}

bool InterpolationBezierCurveC2::RemoveExpiredChildren()
{
	return false;
}

void InterpolationBezierCurveC2::RecalculateIfModified()
{
	if (RemoveExpiredChildren())
	{
		//Recalculate interpolation Points
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

	for (int i = 0; i < interpolationKnots.size() - 1; i++)
	{
		upperDiag.push_back(1.0f);
		lowerDiag.push_back(1.0f);
	}

	// insert the first point
	auto p0 = interpolationKnots[0].lock();
	auto p1 = interpolationKnots[1].lock();
	auto pnPos = p0->m_object->GetPosition();
	auto pnPosPrev = p1->m_object->GetPosition();
	auto diff = XMF3SUB(pnPos, pnPosPrev);
	xVector[0] = 3 * diff.x;
	yVector[0] = 3 * diff.y;
	zVector[0] = 3 * diff.z;

	for (int i = 0; i < interpolationKnots.size() - 2; i++)
	{
		//insert 3(point[i+2] - point[i])
		auto pi = interpolationKnots[i].lock();
		auto pinext = interpolationKnots[i + 2].lock();
		auto pnPos = pi->m_object->GetPosition();
		auto pnPosNext = pinext->m_object->GetPosition();
		diff = XMF3SUB(pnPosNext, pnPos);
		xVector[i + 1] = 3 * diff.x;
		yVector[i + 1] = 3 * diff.y;
		zVector[i + 1] = 3 * diff.z;

	}
	// insert the last point
	auto pnprev = interpolationKnots[interpolationKnots.size() - 2].lock();
	auto pn = interpolationKnots[interpolationKnots.size() - 1].lock();
	pnPos = pn->m_object->GetPosition();
	pnPosPrev = pnprev->m_object->GetPosition();
	diff = XMF3SUB(pnPos, pnPosPrev);
	xVector[interpolationKnots.size() - 1] = 3 * diff.x;
	yVector[interpolationKnots.size() - 1] = 3 * diff.y;
	zVector[interpolationKnots.size() - 1] = 3 * diff.z;


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
		a[i] = ptiPos;
		// b = Di
		b[i] = Di;
		// c = 3 diff - 2Di - Di+1
		c[i] = XMFloat3TimesFloat(diff, 3);
		c[i] = XMF3SUB(c[i], XMFloat3TimesFloat(Di, 2));
		c[i] = XMF3SUB(c[i], Di1);
		// d = -2 diff + Di + Di+1
		d[i] = XMFloat3TimesFloat(diff, -2);
		d[i] = XMF3SUM(c[i], Di);
		d[i] = XMF3SUM(c[i], Di1);
	}

	std::vector<DirectX::XMFLOAT3> resultPos;

	// convert abcd to bernstein basis
	for (int i = 0; i < xRes.size; i++)
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
