#pragma once
#include <vector>
#include "cadStructures.h"

// Represents a hierarchy node used to represent objects in the scene
class Node
{
public:
	std::vector<Node*> children;
	Object* object;
	Node* AttachChild(Object* object);
	void DrawHierarchyNode();
	void Update();
	void Render(RenderData* renderData);
private:
	bool isDirty;
};
