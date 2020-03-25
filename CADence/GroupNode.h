#pragma once
#include <vector>
#include <memory>
#include "Node.h"
#include "Objects.h"

struct GroupNode : Node
{
public:
	explicit GroupNode(std::vector<std::weak_ptr<Node>> m_children);
	// Vector Of Dummy Nodes?

	void AddChild(std::weak_ptr<Node> child);
	void RemoveChild(std::weak_ptr<Node> child);
	void DrawNodeGUI(Scene& scene) override;
private:
	std::vector<std::weak_ptr<Node>> m_children;
	void DrawChildGUI();
};