#include "Trimmer.h"
#include "mathUtils.h"
#include <vector>

void Trimmer::AddCurveToMesh()
{
	//connect the curve vertices into lines and add them to index and vertex buffers
}

void Trimmer::ConnectPointsWithIntersections()
{
	// Ignore the points that have out status and connect those that are "In" with the intersection points
	// If all are in/out there are no intersection points so connect all (in points)
}

void Trimmer::DetermineIntersectedEdges()
{
	// Determines which edges of the parametric rectangle were intersected and store info in CornerInOutSamples
}

// TODO rewerite to find closest intersection with parameter line that is not pt or nextPt
DirectX::XMFLOAT2 Trimmer::FindIntersectionwithLine(DirectX::XMFLOAT2 pt, DirectX::XMFLOAT2 nextPt, float step,  bool affectU)
{
	// Figure out what to do 
	DirectX::XMFLOAT2 res = pt;
	float curDist = INFINITY;

	int maxCell;
	int minCell;
	
	float ptParam;
	float ptNextParam;

	if (affectU)
	{
		ptParam = pt.x;
		ptNextParam = nextPt.x;
	}
	else {
		ptParam = pt.y;
		ptNextParam = nextPt.y;
	}

	float maxParam = max(ptParam, ptNextParam);
	float minParam = min(ptParam, ptNextParam);

	int cell = int(ptParam / step);
	int nextCell = int(ptNextParam / step);
	maxCell = max(cell, nextCell);
	minCell = min(cell, nextCell);

	for (int i = minCell; i <= maxCell; i++)
	{
		float lineU = i * step;
		// The floor of the cells is taken so we must check that the parameter line with lowest parameter is not included
		// because it should be out of domain
		if (lineU > minParam && lineU < maxParam)
		{
			float uFrac = (lineU - ptParam) / (ptNextParam - ptParam);
			auto pointDiff = nextPt - pt;
			auto intersectionPt = pt + pointDiff * uFrac;
			float dist = Dot(intersectionPt - pt, intersectionPt - pt);
			if (dist < curDist && dist > 0.0f)
			{
				res = intersectionPt;
				curDist = dist;
			}
		}
	}	
	
	return res;
}

std::vector<DirectX::XMFLOAT2> Trimmer::IntersectCurveWithGrid(std::vector<DirectX::XMFLOAT2> paramCurve, float Ustep, float Vstep)
{

	// uDist = 1.f / usamples - u length of one sample
	// vDist = 1.f / vsamples - v length of one sample

	// Detect if between the point n and n+1 there should be a point that intersects a constant parameter line and add it to the list between the two points

	auto it = paramCurve.begin();
	auto next = it;
	next++;

	//TODO sometimes Max(cell1,cell2) gives wrong cell when the point is laying on the line
	while (next!= paramCurve.end())
	{
		auto pt = *it;
		auto nextPt = *next;

		float ptU = pt.x;
		float ptV = pt.y;
		float nextPtU = nextPt.x;
		float nextPtV = nextPt.y;

		int uCell = int(ptU / Ustep);
		int vCell = int(ptV / Vstep);
		int nextUCell = (nextPtU / Ustep);
		int nextVCell = (nextPtV / Vstep);
		int deltaCellU = nextUCell - uCell;
		int deltaCellV = nextVCell - vCell;

		bool intersectsULine, intersectsVLine;
		intersectsULine = uCell != nextUCell;
		intersectsVLine = vCell != nextVCell;

		bool inserted = false;
		DirectX::XMFLOAT2 intersectingPoint;

		if (intersectsULine && intersectsVLine)
		{
			// intersect with close line, the added point should intersect with the next point in the second dimension automatically			
			DirectX::XMFLOAT2 uLinePoint = FindIntersectionwithLine(pt, nextPt, Ustep,true);
			DirectX::XMFLOAT2 vLinePoint = FindIntersectionwithLine(pt, nextPt, Vstep,false);

			float uDist = sqrt(Dot(pt - uLinePoint, pt - uLinePoint));
			float vDist = sqrt(Dot(pt - vLinePoint, pt - vLinePoint));

			if (uDist == 0.0f) uDist = INFINITY;
			if (vDist == 0.0f) vDist = INFINITY; //Param space is [0,1] x [0,1] so distance 5 cannot be obtained

			if (uDist < vDist && uDist != INFINITY)
			{
				// Add u point to the vector
				intersectingPoint = uLinePoint;
				inserted = true;
			}
			else if(vDist != INFINITY)
			{
				// Add v point to the vector
				intersectingPoint = vLinePoint;
				inserted = true;
			}
		}
		else {

			if (intersectsULine)
			{
				// there is a U constant param line between pt and NextPt
				auto resPt = FindIntersectionwithLine(pt, nextPt, Ustep, true);
				float uDist = sqrt(Dot(pt - resPt, pt - resPt));
				if (uDist != 0.0f)
				{
					intersectingPoint = resPt;
					inserted = true;
				}
			}
			else if (intersectsVLine)
			{
				// there is a V constant param line between pt and NextPt
				auto resPt = FindIntersectionwithLine(pt, nextPt, Vstep, false);
				float vDist = sqrt(Dot(pt - resPt, pt - resPt));
				if (vDist != 0.0f)
				{
					intersectingPoint = resPt;
					inserted = true;
				}
			}
		}		

		if (inserted == false)
		{
			it++;
			next++;
		}
		else {
			it = paramCurve.insert(next, intersectingPoint);
			next = it;
			next++;
		}
		
	}

	return paramCurve;
}

void Trimmer::Trim(std::vector<DirectX::XMFLOAT2> paramCurve, int uLineCount, int vLineCount)
{
	float uStep = 1.f / (float)(uLineCount - 1);
	float vStep = 1.f / (float)(vLineCount - 1);

	// Add constant param lines intersections to the param grid
	auto modifiedCurve = IntersectCurveWithGrid(paramCurve, uStep, vStep);

	for (float u = 0; u <= uLineCount; u++)
	{
		for (float v = 0; v <= vLineCount; v++)
		{
			// Iterate through the grid and produce the next CornerInOutSamples
			DetermineIntersectedEdges();
			ConnectPointsWithIntersections();
		}
	}

	// Add the whole curve to the vertex and index buffer 
	AddCurveToMesh();
}