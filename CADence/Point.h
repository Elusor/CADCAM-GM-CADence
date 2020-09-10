#pragma once
#include "MeshObject.h"

struct Point : public MeshObject<VertexPositionColor>
{
public:
	Point(float m_size = 0.3f);
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
	void RenderObjectSpecificContextOptions(Scene& scene);
	void UpdateObject() override;

	void Scale(DirectX::XMFLOAT3 scale) override;
	void Rotate(DirectX::XMFLOAT3 scale) override;

private:
	float m_size;
};
