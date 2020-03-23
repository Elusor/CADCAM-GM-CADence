#pragma once
#include <vector>
#include "cadStructures.h"
#include <memory>

class Scene;

// Represents a hierarchy node used to represent objects in the scene
class Node
{
public:
	std::unique_ptr<Object> m_object;

	bool m_isSelected = false;	
	bool m_isRenamed = false;

	void DrawNodeGUI(Scene& scene);

	void Update();
	void Render(std::unique_ptr<RenderData> & renderData);	
};
