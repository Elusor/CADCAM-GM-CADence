#include "Trimmer.h"
#include "mathUtils.h"
#include <vector>
#include <algorithm>

void Trimmer::AddCurveToMesh(std::vector<IndexedVertex> curve, std::vector<unsigned short>& indices)
{
	//connect the curve vertices into lines and add them to index and vertex buffers
	// TODO WATCH OUT FOR THE LAST POINT
	for (int i = 0; i < curve.size()-1; i++)
	{
		indices.push_back(curve[i].index);
		indices.push_back(curve[i+1].index);
	}
}

void Trimmer::ConnectPointsWithIntersections()
{
	// Ignore the points that have out status and connect those that are "In" with the intersection points
	// If all are in/out there are no intersection points so connect all (in points)
}

// Vertices are indexed by u first and then v, so idx = umax*v + u
void Trimmer::DetermineIntersectedEdges(
	int curU, int curV, int maxU, int maxV, SampleInOutStatus** samples,
	std::vector<IndexedVertex> intersectingPoints, std::vector<unsigned short>& indices)
{
	// Determines which edges of the parametric rectangle were intersected and store info in CornerInOutSamples
	// If and edge has %2 points do not change the status of the edges but connect them to the intersecting edges
	float uStep = 1.f / (float)(maxU - 1);
	float vStep = 1.f / (float)(maxV - 1);
	int index = maxU * curV + curU;
	int rightIndex = index + 1;
	int downIndex = index + maxU;	
	DirectX::XMFLOAT2 curPos = DirectX::XMFLOAT2(uStep * curU, vStep * curV);
	DirectX::XMFLOAT2 rightPos = DirectX::XMFLOAT2(uStep * (curU + 1.f), vStep * curV);
	DirectX::XMFLOAT2 downPos = DirectX::XMFLOAT2(uStep * curU, vStep * (curV + 1.f));
	auto curStatus = samples[curU][curV];

	if (intersectingPoints.empty())
	{
		// Propagate status - no intersections means that no vertex can leave/enter the area
		samples[curU + 1][curV] = samples[curU][curV];
		samples[curU][curV + 1] = samples[curU][curV];
		if (curStatus == SampleInOutStatus::Out)
		{
			// upper left to upper right
			indices.push_back(index);
			indices.push_back(index + 1);
			// upper left to lower left
			indices.push_back(index);
			indices.push_back(index + maxU);
		}		
	}
	else
	{
		int rightCount = 0, downCount = 0;
		std::vector<IndexedVertex> rightPoints, downPoints;

		rightPoints.push_back(IndexedVertex{ curPos, index });
		downPoints.push_back(IndexedVertex{ curPos, index });

		for (int i = 0; i < intersectingPoints.size(); i++)
		{
			// count intersections on each edge
			auto pt = intersectingPoints[i];
			if (pt.params.x > curPos.x)
			{
				rightPoints.push_back(pt);
			}
			else {				
				downPoints.push_back(pt);
			}
		}
		rightPoints.push_back({ rightPos, rightIndex });
		downPoints.push_back({ downPos, downIndex });

		rightCount = rightPoints.size();
		downCount = downPoints.size();
		
		// Sort both lists in ascending order by the parameter
		std::sort(rightPoints.begin(), rightPoints.end(), [](IndexedVertex a, IndexedVertex b) {return a.params.x < b.params.x; });
		std::sort(downPoints.begin(), downPoints.end(), [](IndexedVertex a, IndexedVertex b) {return a.params.y < b.params.y; });

		// construct all pair across edges
		std::vector<XMINT2> rightLines, downLines;
		for (int i = 0; i < rightPoints.size()-1; i++)
		{
			rightLines.push_back(DirectX::XMINT2(
				rightPoints[i].index,
				rightPoints[i + 1].index));
		}

		for (int i = 0; i < downPoints.size() - 1; i++)
		{
			downLines.push_back(DirectX::XMINT2(
				downPoints[i].index,
				downPoints[i + 1].index));
		}

		// based on main sample status take even or odd lines
		// If the corner is in take the first, third, ... [odd] one next else take the 0th, 2nd, ... [even] ones
		if (curStatus == Out)
		{
			// Add even indices of lines lists to the indices vector
			for (int i = 0; i < rightLines.size(); i += 2)
			{
				indices.push_back(rightLines[i].x); 
				indices.push_back(rightLines[i].y);
			}

			for (int i = 0; i < downLines.size(); i += 2)
			{
				indices.push_back(downLines[i].x);
				indices.push_back(downLines[i].y);
			}
		}
		else {
			// Add odd indices of lines lists to the indices vector
			for (int i = 1; i < rightLines.size(); i += 2)
			{
				indices.push_back(rightLines[i].x);
				indices.push_back(rightLines[i].y);
			}

			for (int i = 1; i < downLines.size(); i += 2)
			{
				indices.push_back(downLines[i].x);
				indices.push_back(downLines[i].y);
			}
		}

		if (rightCount % 2 != 0){
			samples[curU + 1][curV] = GetOppositeStatus(curStatus);
		}
		else {
			samples[curU + 1][curV] = curStatus;
		}

		if (downCount % 2 != 0){			
			samples[curU][curV + 1] = GetOppositeStatus(curStatus);
		}
		else {
			samples[curU][curV + 1] = curStatus;
		}
	}
}

#pragma region edgeCases
void Trimmer::DetermineIntersectedEdgesLastRow(int curU, int curV, int maxU, int maxV, SampleInOutStatus** samples, std::vector<IndexedVertex> intersectingPoints, std::vector<unsigned short>& indices)
{
	// Determines which edges of the parametric rectangle were intersected and store info in CornerInOutSamples
	// If and edge has %2 points do not change the status of the edges but connect them to the intersecting edges
	float uStep = 1.f / (float)(maxU - 1);
	float vStep = 1.f / (float)(maxV - 1);
	int index = maxU * curV + curU;
	int rightIndex = index + 1;
	DirectX::XMFLOAT2 curPos = DirectX::XMFLOAT2(uStep * curU, vStep * curV);
	DirectX::XMFLOAT2 rightPos = DirectX::XMFLOAT2(uStep * (curU + 1.f), vStep * curV);
	auto curStatus = samples[curU][curV];

	if (intersectingPoints.empty())
	{
		// Propagate status - no intersections means that no vertex can leave/enter the area
		samples[curU + 1][curV] = samples[curU][curV];
		if (curStatus == SampleInOutStatus::Out)
		{
			// upper left to upper right
			indices.push_back(index);
			indices.push_back(index + 1);
		}
	}
	else
	{
		int rightCount = 0;
		std::vector<IndexedVertex> rightPoints;

		rightPoints.push_back(IndexedVertex{ curPos, index });

		for (int i = 0; i < intersectingPoints.size(); i++)
		{
			// count intersections on each edge
			auto pt = intersectingPoints[i];
			if (pt.params.x > curPos.x)
			{
				rightPoints.push_back(pt);
			}
			
		}
		rightPoints.push_back({ rightPos, rightIndex });

		rightCount = rightPoints.size();

		// Sort both lists in ascending order by the parameter
		std::sort(rightPoints.begin(), rightPoints.end(), [](IndexedVertex a, IndexedVertex b) {return a.params.x < b.params.x; });

		// construct all pair across edges
		std::vector<XMINT2> rightLines, downLines;
		for (int i = 0; i < rightPoints.size() - 1; i++)
		{
			rightLines.push_back(DirectX::XMINT2(
				rightPoints[i].index,
				rightPoints[i + 1].index));
		}

		// based on main sample status take even or odd lines
		if (curStatus == Out)
		{
			// Add even indices of lines lists to the indices vector
			for (int i = 0; i < rightLines.size(); i += 2)
			{
				indices.push_back(rightLines[i].x);
				indices.push_back(rightLines[i].y);
			}

		}
		else {
			// Add odd indices of lines lists to the indices vector
			for (int i = 1; i < rightLines.size(); i += 2)
			{
				indices.push_back(rightLines[i].x);
				indices.push_back(rightLines[i].y);
			}

		}

		if (rightCount % 2 != 0) {
			samples[curU + 1][curV] = GetOppositeStatus(curStatus);
		}
		else {
			samples[curU + 1][curV] = curStatus;
		}
	}
}

void Trimmer::DetermineIntersectedEdgesLastColumn(int curU, int curV, int maxU, int maxV, SampleInOutStatus** samples, std::vector<IndexedVertex> intersectingPoints, std::vector<unsigned short>& indices)
{
	// Determines which edges of the parametric rectangle were intersected and store info in CornerInOutSamples
	// If and edge has %2 points do not change the status of the edges but connect them to the intersecting edges
	float uStep = 1.f / (float)(maxU - 1);
	float vStep = 1.f / (float)(maxV - 1);
	int index = maxU * curV + curU;
	int downIndex = index + maxU;
	DirectX::XMFLOAT2 curPos = DirectX::XMFLOAT2(uStep * curU, vStep * curV);
	DirectX::XMFLOAT2 downPos = DirectX::XMFLOAT2(uStep * curU, vStep * (curV + 1.f));
	auto curStatus = samples[curU][curV];

	if (intersectingPoints.empty())
	{
		// Propagate status - no intersections means that no vertex can leave/enter the area
		samples[curU][curV + 1] = samples[curU][curV];
		if (curStatus == SampleInOutStatus::Out)
		{
			// upper left to lower left
			indices.push_back(index);
			indices.push_back(index + maxU);
		}
	}
	else
	{
		int downCount = 0;
		std::vector<IndexedVertex> rightPoints, downPoints;

		downPoints.push_back(IndexedVertex{ curPos, index });

		for (int i = 0; i < intersectingPoints.size(); i++)
		{
			// count intersections on each edge
			auto pt = intersectingPoints[i];
			if (pt.params.y > curPos.y)
			{
				downPoints.push_back(pt);
			}
			
		}
		downPoints.push_back({ downPos, downIndex });

		downCount = downPoints.size();

		// Sort both lists in ascending order by the parameter
		std::sort(downPoints.begin(), downPoints.end(), [](IndexedVertex a, IndexedVertex b) {return a.params.y < b.params.y; });

		// construct all pair across edges
		std::vector<XMINT2>downLines;

		for (int i = 0; i < downPoints.size() - 1; i++)
		{
			downLines.push_back(DirectX::XMINT2(
				downPoints[i].index,
				downPoints[i + 1].index));
		}

		// based on main sample status take even or odd lines
		if (curStatus == Out)
		{

			for (int i = 0; i < downLines.size(); i += 2)
			{
				indices.push_back(downLines[i].x);
				indices.push_back(downLines[i].y);
			}
		}
		else {
			// Add odd indices of lines lists to the indices vector

			for (int i = 1; i < downLines.size(); i += 2)
			{
				indices.push_back(downLines[i].x);
				indices.push_back(downLines[i].y);
			}
		}

		if (downCount % 2 != 0) {
			samples[curU][curV + 1] = GetOppositeStatus(curStatus);
		}
		else {
			samples[curU][curV + 1] = curStatus;
		}
	}
}
#pragma endregion

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

std::vector<IndexedVertex> Trimmer::IntersectCurveWithGrid(std::vector<IndexedVertex>& paramCurve, float Ustep, float Vstep)
{

	// uDist = 1.f / usamples - u length of one sample
	// vDist = 1.f / vsamples - v length of one sample

	// Detect if between the point n and n+1 there should be a point that intersects a constant parameter line and add it to the list between the two points
	std::vector<IndexedVertex> addedPoints;
	auto it = paramCurve.begin();
	auto next = it;
	next++;

	int i = 0;
	//TODO sometimes Max(cell1,cell2) gives wrong cell when the point is laying on the line
	while (next!= paramCurve.end())
	{
		auto pt3 = *it;
		auto nextPt3 = *next;
		auto pt = pt3.params;
		auto nextPt = nextPt3.params;

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

		// TODO: Check if the point lies exactly on the line intersection

		if (intersectsULine && intersectsVLine)
		{

			// intersect with close line, the added point should intersect with the next point in the second dimension automatically			
			DirectX::XMFLOAT2 uLinePoint = FindIntersectionwithLine(pt, nextPt, Ustep, true);
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
			it->index = i++;
			it++;
			next++;
		}
		else {
			it->index = i;
			auto IdxPt = IndexedVertex{ intersectingPoint, i+1 };
			it = paramCurve.insert(next, IdxPt);
			addedPoints.push_back(IdxPt);
			i++;
			next = it;
			next++;
		}
		
	}
	// Last point index
	it->index = i++;
	return addedPoints;
}

TrimmedSpace Trimmer::Trim(std::vector<DirectX::XMFLOAT2> paramCurve, int uLineCount, int vLineCount, SampleInOutStatus trimmedSide)
{

	//paramCurve = { {0.375f, 0.125f}, {0.625f, 0.625f}, {0.375f, 0.875f}, {0.375f, 0.125f} };
	// TODO WATCH OUT FOR POINTS THAT ARE EXACTLY ON THE OLD GRID ex. 0.5,0.5 etc.
	float uStep = 1.f / (float)(uLineCount - 1);
	float vStep = 1.f / (float)(vLineCount - 1);

	std::vector<unsigned short> indices;
	std::vector<DirectX::XMFLOAT2> vertices;
	SampleInOutStatus** samples = new SampleInOutStatus*[uLineCount];
	for (int i = 0; i < uLineCount; i++)
	{
		samples[i] = new SampleInOutStatus[vLineCount];
	}
	samples[0][0] = trimmedSide;

	std::vector<IndexedVertex> indexedCurve;
	for (int i = 0; i < paramCurve.size(); i++)
	{
		indexedCurve.push_back(IndexedVertex{ paramCurve[i], 0 });
	}

	// Add constant param lines intersections to the param grid
	auto addedPoints = IntersectCurveWithGrid(indexedCurve, uStep, vStep);
	int startingCurveIndex = uLineCount * vLineCount;

	for (int i = 0; i < indexedCurve.size(); i++)
	{
		indexedCurve[i].index += startingCurveIndex;
	}

	for (int i = 0; i < addedPoints.size(); i++)
	{
		addedPoints[i].index += startingCurveIndex;
	}

	// Vertex Lists are prepared
	// Calculate for the wholegrid without the last edge and column
	for (float u = 0; u < uLineCount-1; u++)
	{
		for (float v = 0; v < vLineCount-1; v++)
		{
			int upperLeftIdx = u * v;			
			float lowerU = u * uStep;
			float lowerV = v * vStep;

			std::vector<IndexedVertex> candidatePoints;
			for (int i = 0; i < indexedCurve.size(); i++)
			{
				auto pt = indexedCurve[i];
				float uDiff = pt.params.x - lowerU;
				float vDiff = pt.params.y - lowerV;

				
				if (uDiff < uStep && vDiff < vStep &&
					uDiff >= 0.0f && vDiff >= 0.0f && 
					(uDiff == 0.0f || vDiff == 0.0f))
				{
					candidatePoints.push_back(pt);
				}
			}

			// Iterate through the grid and produce the next CornerInOutSamples
			DetermineIntersectedEdges(u, v, uLineCount, vLineCount, samples, candidatePoints, indices);
		}
	}
	
	// Determine for last row and last column
	for (int u = 0; u < uLineCount -1; u++)
	{
		float lowerU = u * uStep;
		float lowerV = 1.0f;
		std::vector<IndexedVertex> candidatePoints;
		for (int i = 0; i < addedPoints.size(); i++)
		{
			auto pt = addedPoints[i];
			float uDiff = pt.params.x - lowerU;
			float vDiff = pt.params.y - lowerV;

			if (uDiff < uStep && vDiff < vStep &&
				uDiff >= 0.0f && vDiff >= 0.0f)
			{
				candidatePoints.push_back(pt);
			}
		}

		DetermineIntersectedEdgesLastRow(u, vLineCount-1, uLineCount, vLineCount, samples, candidatePoints, indices);
	}

	for (int v = 0; v < vLineCount -1; v++)
	{
		float lowerU = 1.0f;
		float lowerV = v * uStep;
		std::vector<IndexedVertex> candidatePoints;
		for (int i = 0; i < addedPoints.size(); i++)
		{
			auto pt = addedPoints[i];
			float uDiff = pt.params.x - lowerU;
			float vDiff = pt.params.y - lowerV;

			if (uDiff < uStep && vDiff < vStep &&
				uDiff >= 0.0f && vDiff >= 0.0f)
			{
				candidatePoints.push_back(pt);
			}
		}

		DetermineIntersectedEdgesLastColumn(uLineCount -1 , v, uLineCount, vLineCount, samples, candidatePoints, indices);
	}

	// Add the whole curve to the index buffer 
	AddCurveToMesh(indexedCurve, indices);

	// Create vertex data
	for (int v = 0; v < vLineCount; v++)
	{
		for (int u = 0; u < uLineCount; u++)
		{
			float uVal = u * uStep;
			float vVal = v * vStep;
			vertices.push_back(DirectX::XMFLOAT2(uVal, vVal));
		}
	}

	for (int i = 0; i < indexedCurve.size(); i++)
	{
		vertices.push_back(indexedCurve[i].params);
	}

	//Cleanup
	for (int i = 0; i < uLineCount; i++)
	{
		delete[] samples[i];
	}
	delete[] samples;

	TrimmedSpace ts;
	ts.indices = indices;
	ts.vertices = vertices;
	return ts;
}

SampleInOutStatus Trimmer::GetOppositeStatus(SampleInOutStatus status)
{
	if (status == SampleInOutStatus::In)
	{
		return SampleInOutStatus::Out;
	}
	else
	{
		return SampleInOutStatus::In;
	}
}
