#pragma once
#include "Object.h"
#include "vertexStructures.h"
#include <xmemory>

struct SceneGrid : Object
{
public:
	SceneGrid();
	SceneGrid(int gridSize);

	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	bool CreateParamsGui() override;
	void UpdateObject() override;

private:

	void GenerateVertices(int gridSize);

	int m_gridSize;
	DirectX::XMFLOAT3 m_color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	std::vector<VertexPositionColor> m_vertices;
	std::vector<unsigned short> m_indices;
};
