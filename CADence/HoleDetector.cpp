#include "HoleDetector.h"
#include "BezierPatch.h"
std::weak_ptr<Node> ValidConnection(std::vector<std::weak_ptr<Node>> edge1, std::vector<std::weak_ptr<Node>> edge2)
{
	std::weak_ptr<Node> duplicateVertex;

	int vertexCount = 4;
	auto e1pt1 = edge1[0].lock();
	auto e1pt2 = edge1[3].lock();

	auto e2pt1 = edge2[0].lock();
	auto e2pt2 = edge2[3].lock();

	if (e1pt1 == e2pt2)
	{
		duplicateVertex = e1pt1;
		vertexCount--;
	}
	if (e1pt1 == e2pt1)
	{
		duplicateVertex = e1pt1;
		vertexCount--;
	}
	if (e1pt2 == e2pt2)
	{
		duplicateVertex = e1pt2;
		vertexCount--;
	}
	if (e1pt2 == e2pt1)
	{
		duplicateVertex = e1pt2;
		vertexCount--;
	}

	return vertexCount == 3 ? duplicateVertex : std::weak_ptr<Node>();
}

HoleData HoleDetector::DetectHole(BezierPatch* p1, BezierPatch* p2, BezierPatch* p3)
{	

	HoleData hole;
	// iterate throught the edges
	for (int e1 = 0; e1 < 4; e1++)
	{
		for (int e2 = 0; e2 < 4; e2++)
		{
			auto edge1 = p1->GetPoints((BoundaryDirection)e1);
			auto edge2 = p2->GetPoints((BoundaryDirection)e2);
			auto cp = ValidConnection(edge1, edge2);
			if (cp.expired() == false)
			{
				for (int e3 = 0; e3 < 4; e3++)
				{
					auto edge3 = p3->GetPoints((BoundaryDirection)e3);
					auto cp1 = ValidConnection(edge1, edge3);
					auto cp2 = ValidConnection(edge2, edge3);
					if (cp1.expired() == false && cp2.expired() == false)
					{
						if (cp1.lock() != cp.lock() && cp2.lock() != cp.lock())
						{
							hole.isValid = true;
							hole.p1 = p1;
							hole.p2 = p2;
							hole.p3 = p3;
							hole.p1Edge = (BoundaryDirection)e1;
							hole.p2Edge = (BoundaryDirection)e2;
							hole.p3Edge = (BoundaryDirection)e3;
						}
					}

				}
			}
		}
	}

	return hole;
}