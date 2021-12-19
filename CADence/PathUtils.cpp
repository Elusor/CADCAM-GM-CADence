#include "PathUtils.h"
#include "mathUtils.h"

std::string PrepareMoveInstruction(DirectX::SimpleMath::Vector3 pos)
{
	std::stringstream stream;
	stream << "G01";

	// Prepare the number formatting
	stream << std::fixed << std::showpoint << std::setprecision(3);
	stream << "X" << pos.x;
	stream << "Y" << pos.y;
	stream << "Z" << pos.z;

	return stream.str();
}

DirectX::XMFLOAT3 ConvertToMilimeters(DirectX::XMFLOAT3 point)
{
	DirectX::XMFLOAT3 res;
	res = DirectX::XMFLOAT3(point.x * 10.f, point.y * 10.f, point.z * 10.f);
	return res;
}

std::vector<LineIntersectionData> IntersectCurves(const std::vector<DirectX::XMFLOAT2>& params1, const std::vector<DirectX::XMFLOAT2>& params2)
{
	std::vector<LineIntersectionData> result;

	for (size_t line1Idx = 0; line1Idx < params1.size() - 1; line1Idx++)
	{
		auto beg1 = params1[line1Idx];
		auto end1 = params1[line1Idx + 1];

		for (size_t line2Idx = 0; line2Idx < params2.size() - 1; line2Idx++)
		{
			auto beg2 = params2[line2Idx];
			auto end2 = params2[line2Idx + 1];

			auto res = GetIntersectionPoint(beg1, end1, beg2, end2);
			if (res.first)
			{
				LineIntersectionData intersectionPoint;
				intersectionPoint.qLineIndex = line1Idx;
				intersectionPoint.pLineIndex = line2Idx;
				intersectionPoint.intersectionPoint = res.second;
				result.push_back(intersectionPoint);
			}
		}
	}

	return result;
}

std::vector<LineIntersectionData> IntersectCurves(
	const std::vector<DirectX::XMFLOAT3>& params1,
	const std::vector<DirectX::XMFLOAT3>& params2)
{
	std::vector<LineIntersectionData> result;

	for (size_t line1Idx = 0; line1Idx < params1.size() - 1; line1Idx++)
	{
		auto beg1 = params1[line1Idx];
		auto end1 = params1[line1Idx + 1];

		for (size_t line2Idx = 0; line2Idx < params2.size() - 1; line2Idx++)
		{
			auto beg2 = params2[line2Idx];
			auto end2 = params2[line2Idx + 1];

			auto res = GetIntersectionPoint(
				DirectX::XMFLOAT2(beg1.x, beg1.y),
				DirectX::XMFLOAT2(end1.x, end1.y),
				DirectX::XMFLOAT2(beg2.x, beg2.y),
				DirectX::XMFLOAT2(end2.x, end2.y));

			if (res.first)
			{
				LineIntersectionData intersectionPoint;
				intersectionPoint.qLineIndex = line1Idx;
				intersectionPoint.pLineIndex = line2Idx;
				intersectionPoint.intersectionPoint = res.second;
				result.push_back(intersectionPoint);
			}
		}
	}

	return result;
}

std::pair<bool, DirectX::XMFLOAT2> GetIntersectionPoint(
	const DirectX::SimpleMath::Vector2& beg1,
	const DirectX::SimpleMath::Vector2& end1,
	const DirectX::SimpleMath::Vector2& beg2,
	const DirectX::SimpleMath::Vector2& end2)
{
	auto res = std::pair<bool, DirectX::XMFLOAT2>();
	res.first = false;

	auto segment1Vector = end1 - beg1;
	auto segment2Vector = end2 - beg2;

	auto segment1ToBeg2 = beg2 - beg1;
	auto segment1ToEnd2 = end2 - beg1;

	auto segment2ToBeg1 = beg1 - beg2;
	auto segment2ToEnd1 = end1 - beg2;

	auto cross1_beg2 = Cross(segment1Vector, segment1ToBeg2);
	auto cross1_end2 = Cross(segment1Vector, segment1ToEnd2);

	auto cross2_beg1 = Cross(segment2Vector, segment2ToBeg1);
	auto cross2_end1 = Cross(segment2Vector, segment2ToEnd1);

	auto twoOnDifferentSidesOfOne = Dot(cross1_beg2, cross1_end2) <= 0;
	auto oneOnDifferentSidesOfTwo = Dot(cross2_beg1, cross2_end1) <= 0;

	res.first = oneOnDifferentSidesOfTwo && twoOnDifferentSidesOfOne;

	if (res.first)
	{
		const float s1_x = end1.x - beg1.x;
		const float s1_y = end1.y - beg1.y;

		const float s2_x = end2.x - beg2.x;
		const float s2_y = end2.y - beg2.y;

		// https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect		
		const float t =
			(s2_x * (beg1.y - beg2.y) - s2_y * (beg1.x - beg2.x)) /
			(-s2_x * s1_y + s1_x * s2_y);

		assert(t >= 0.0f && t <= 1.0f);
		res.second = beg1 + t * (end1 - beg1);
	}

	return res;
}