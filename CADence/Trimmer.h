#pragma once
#include <DirectXMath.h>
#include <vector>
enum SampleInOutStatus
{
	In,
	Out
};

// Assumes that when viewed - the parametric space is 
// u [0,1] from left to right
// v [0,1] from top to bottom
struct CornerInOutSamples
{
	SampleInOutStatus center;
	SampleInOutStatus bot;
	SampleInOutStatus right;
};

struct IndexedVertex
{
	DirectX::XMFLOAT2 params;
	int index;
};

class Trimmer
{
public:
	void AddCurveToMesh(std::vector<IndexedVertex> curve, std::vector<unsigned short>& indices);
	void ConnectPointsWithIntersections();
	void DetermineIntersectedEdges( 
		int curU, int curV, int maxU, int maxV, SampleInOutStatus** samples,
		std::vector<IndexedVertex> intersectingPoints, std::vector<unsigned short>& indices);
	DirectX::XMFLOAT2 FindIntersectionwithLine(
		DirectX::XMFLOAT2 pt, DirectX::XMFLOAT2 nextPt, 
		float step, bool affectU);
	std::vector<IndexedVertex> IntersectCurveWithGrid(std::vector<IndexedVertex>& paramCurve, float Ustep, float Vstep);
	void Trim(std::vector<DirectX::XMFLOAT2> paramCurve, int uLineCount, int vLineCount);
private:
	SampleInOutStatus GetOppositeStatus(SampleInOutStatus status);
};
