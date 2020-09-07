#pragma once
#include <DirectXMath.h>
#include <vector>
enum SampleInOutStatus
{
	Out,
	In
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

struct TrimmedSpace
{
	std::vector<unsigned short> indices;
	std::vector<DirectX::XMFLOAT2> vertices;
};

static class Trimmer
{
public:
	static TrimmedSpace Trim(std::vector<DirectX::XMFLOAT2> paramCurve, int uLineCount, int vLineCount, SampleInOutStatus remainingSide);
	static SampleInOutStatus GetOppositeStatus(SampleInOutStatus status);
private:
	static void AddCurveToMesh(std::vector<IndexedVertex> curve, std::vector<unsigned short>& indices);
	static void ConnectPointsWithIntersections();
	static void DetermineIntersectedEdges(
		int curU, int curV, int maxU, int maxV, SampleInOutStatus** samples,
		std::vector<IndexedVertex> intersectingPoints, std::vector<unsigned short>& indices);
	static void DetermineIntersectedEdgesLastRow(
		int curU, int curV, int maxU, int maxV, SampleInOutStatus** samples,
		std::vector<IndexedVertex> intersectingPoints, std::vector<unsigned short>& indices);
	static void DetermineIntersectedEdgesLastColumn(
		int curU, int curV, int maxU, int maxV, SampleInOutStatus** samples,
		std::vector<IndexedVertex> intersectingPoints, std::vector<unsigned short>& indices);

	static DirectX::XMFLOAT2 FindIntersectionwithLine(
		DirectX::XMFLOAT2 pt, DirectX::XMFLOAT2 nextPt,
		float step, bool affectU);
	static std::vector<IndexedVertex> IntersectCurveWithGrid(std::vector<IndexedVertex>& paramCurve, float Ustep, float Vstep);
};
