#pragma once
#include <vector>
#include <memory>
#include "Node.h"
#include "Objects.h"

struct GroupNode : Node
{
public:

	// Vector Of Dummy Nodes?
	std::vector<std::weak_ptr<Node>> m_children;

	void DrawNodeGUI(Scene& scene) override;
private:
	void DrawChildGUI();
};