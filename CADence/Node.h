#pragma once
#include <vector>
#include "cadStructures.h"

// Represents a hierarchy node used to represent objects in the scene
class Node
{
public:

	std::vector<Node*> m_children;
	Node* m_parent;
	Object* m_object;

	Node* AttachChild(Object* object);
	void DetachFromParent();

	void DrawHierarchyNode();
	void Update();
	void Render(RenderData* renderData);	

private:
	bool isDirty;

	void DrawPopupContextItem(Node* node);
	void Remove();
};
