#pragma once
#include <string>
#include <DirectXMath.h>
#include "SimpleMath.h"
#include <sstream>
#include <iomanip>
#include <vector>

struct LineIntersectionData
{
	int qLineIndex;
	int pLineIndex;
	DirectX::XMFLOAT2 intersectionPoint;
};

std::string PrepareMoveInstruction(DirectX::SimpleMath::Vector3 pos);

DirectX::XMFLOAT3 ConvertToMilimeters(DirectX::XMFLOAT3 point);

std::vector<LineIntersectionData> IntersectCurves(
	const std::vector<DirectX::XMFLOAT2>& params1,
	const std::vector<DirectX::XMFLOAT2>& params2);

std::vector<LineIntersectionData> IntersectCurves(
	const std::vector<DirectX::XMFLOAT3>& params1,
	const std::vector<DirectX::XMFLOAT3>& params2);

std::pair<bool, DirectX::XMFLOAT2> GetIntersectionPoint(
	const DirectX::SimpleMath::Vector2& beg1,
	const DirectX::SimpleMath::Vector2& end1,
	const DirectX::SimpleMath::Vector2& beg2,
	const DirectX::SimpleMath::Vector2& end2);