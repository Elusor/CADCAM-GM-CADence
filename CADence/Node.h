#pragma once
#include <vector>
#include "cadStructures.h"
#include <memory>

// Represents a hierarchy node used to represent objects in the scene
class Node
{
public:

	std::vector<std::shared_ptr<Node>> m_children;
	Node* m_parent;
	std::unique_ptr<Object> m_object;

	std::shared_ptr<Node> AttachChild(std::unique_ptr<Object>& object);
	void DetachFromParent();

	void DrawHierarchyNode();
	void Update();
	void Render(std::unique_ptr<RenderData> & renderData);	

	std::shared_ptr<Node> GetPointerToSelf();

private:
	bool isDirty;

	void DrawPopupContextItem();
	void Remove();
};
