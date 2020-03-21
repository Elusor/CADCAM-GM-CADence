#pragma once
#include <vector>
#include "cadStructures.h"
#include <memory>

// Represents a hierarchy node used to represent objects in the scene
class Node
{
public:
	std::unique_ptr<Object> m_object;

	bool m_isSelected = false;	
	bool m_isRenamed = false;

	void Update();
	void Render();	
};
