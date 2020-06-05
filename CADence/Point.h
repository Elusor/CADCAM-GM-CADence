#pragma once
#include "MeshObject.h"

struct Point : public MeshObject
{
public:
	Point(float m_size = 0.3f);
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
	void RenderObjectSpecificContextOptions(Scene& scene);
	void UpdateObject() override;

private:
	float m_size;
};
