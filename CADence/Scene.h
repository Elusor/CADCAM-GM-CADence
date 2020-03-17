#pragma once
#include <vector>
#include "cadStructures.h"
#include "Node.h"

// Maybe split node into GroupingNodes and Objects node, 
// where grouping nodes are for storing children only and ObjectNodes hold pointers to objects
class Scene {
public:	
	std::shared_ptr<Node> m_selectedNode;
	std::shared_ptr<Node> AttachObject(std::unique_ptr<Object>& object);

	void DrawSceneHierarchy();
	void UpdateScene();
	void RenderScene(std::unique_ptr<RenderData> & renderData);
	void UpdateSelectedNode();

private:
	Node rootNode;

};
