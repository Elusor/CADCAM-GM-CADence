#pragma once
#include <DirectXMath.h>
#include <vector>
enum SampleInOutStatus
{
	In,
	Out
};

struct CornerInOutSamples
{
	SampleInOutStatus upperL;
	SampleInOutStatus upperR;
	SampleInOutStatus lowerL;
	SampleInOutStatus lowerR;
};


class Trimmer
{
public:
	void AddCurveToMesh();
	void ConnectPointsWithIntersections();
	void DetermineIntersectedEdges();
	DirectX::XMFLOAT2 FindIntersectionwithLine(DirectX::XMFLOAT2 pt, DirectX::XMFLOAT2 nextPt, float step, bool affectU);
	std::vector<DirectX::XMFLOAT2> IntersectCurveWithGrid(std::vector<DirectX::XMFLOAT2> paramCurve, float Ustep, float Vstep);
	void Trim(std::vector<DirectX::XMFLOAT2> paramCurve, int uLineCount, int vLineCount);
private:

};
