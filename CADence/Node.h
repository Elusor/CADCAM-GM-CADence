#pragma once
#include <vector>
#include "cadStructures.h"
#include <memory>

// Represents a hierarchy node used to represent objects in the scene
class Node
{
public:
	std::unique_ptr<Object> m_object;

	void Update();
	void Render(std::unique_ptr<RenderData> & renderData, bool selected = false);	
};
