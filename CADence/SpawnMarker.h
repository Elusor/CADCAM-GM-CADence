#pragma once
#include "Object.h"

struct SpawnMarker : Object
{
public:
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
private:
	DirectX::XMFLOAT3 m_color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
};


