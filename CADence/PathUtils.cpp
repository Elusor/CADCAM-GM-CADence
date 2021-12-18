#include "PathUtils.h"

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