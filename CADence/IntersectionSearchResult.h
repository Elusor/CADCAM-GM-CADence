#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>

struct IntersectionSearchResult
{
	std::vector<DirectX::XMFLOAT2> surfQParamsList;
	std::vector<DirectX::XMFLOAT2> surfPParamsList;

	bool m_qIntersectionClosed;
	bool m_pIntersectionClosed;
};