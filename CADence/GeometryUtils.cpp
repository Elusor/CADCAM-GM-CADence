#include <cassert>
#include "mathUtils.h"
#include "GeometryUtils.h"
#include "Node.h"
#include "Transform.h"
#include "Object.h"

XMFLOAT3 DeCastel(XMFLOAT3 v1, XMFLOAT3 v2, float t)
{
	return  (1.f - t) * v1 + t * v2;
}

DividedCurve* DivideBernsteinCurve(std::vector<std::weak_ptr<Node>> bernsteinCurve, float point)
{
	assert(bernsteinCurve.size() == 4);

	float t = point;
	float u = 1.f - t;

	DividedCurve* divCurve = new DividedCurve();
	XMFLOAT3 it0_0, it0_1, it0_2, it0_3;
	XMFLOAT3 it1_0, it1_1, it1_2;
	XMFLOAT3 it2_0, it2_1;
	XMFLOAT3 it3_0;

	it0_0 = bernsteinCurve[0].lock()->m_object->GetPosition(); 
	it0_1 = bernsteinCurve[1].lock()->m_object->GetPosition(); 
	it0_2 = bernsteinCurve[2].lock()->m_object->GetPosition();
	it0_3 = bernsteinCurve[3].lock()->m_object->GetPosition();

	it1_0 = DeCastel(it0_0, it0_1, t);
	it1_1 = DeCastel(it0_1, it0_2, t);
	it1_2 = DeCastel(it0_2, it0_3, t);

	it2_0 = DeCastel(it1_0, it1_1, t);
	it2_1 = DeCastel(it1_1, it1_2, t);

	it3_0 = DeCastel(it2_0, it2_1, t);

	divCurve->begMid.push_back(it0_0);
	divCurve->begMid.push_back(it1_0);
	divCurve->begMid.push_back(it2_0);
	divCurve->begMid.push_back(it3_0);

	divCurve->midEnd.push_back(it3_0);
	divCurve->midEnd.push_back(it2_1);
	divCurve->midEnd.push_back(it1_2);
	divCurve->midEnd.push_back(it0_3);	

	return divCurve;
}
