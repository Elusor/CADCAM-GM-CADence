#pragma once
#include <vector>
#include <memory>
#include "Node.h"
#include "Objects.h"

struct GroupNode : Node
{
public:
	GroupNode();
	explicit GroupNode(std::vector<std::weak_ptr<Node>> m_children);
	// Vector Of Dummy Nodes?

	void AddChild(std::weak_ptr<Node> child);
	void RemoveChild(std::weak_ptr<Node> child);

	void RemoveExpiredChildren();
	void SetChildren(std::vector<std::weak_ptr<Node>> children);
	void SwapReferences(std::weak_ptr<Node> existingChild, std::weak_ptr<Node> newChild);
	void DrawNodeGUI(Scene& scene) override;

	std::vector<std::weak_ptr<Node>> GetSelectedChildren() override;
	std::vector<std::weak_ptr<Node>> GetChildren() override;
	void ClearChildrenSelection() override;
private:
	std::vector<std::weak_ptr<Node>> m_children;
	void DrawChildGUI();
};