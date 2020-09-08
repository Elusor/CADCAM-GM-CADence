#pragma once
#include <vector>
#include <DirectXMath.h>
#include "LoopData.h"


struct IntersectionSearchResultOneDir
{
	std::vector<DirectX::XMFLOAT2> surfQParamsList;
	std::vector<DirectX::XMFLOAT2> surfPParamsList;

	LoopData m_loopData;
};