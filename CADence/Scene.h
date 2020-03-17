#pragma once
#include <vector>
#include "cadStructures.h"
#include "Node.h"

// Maybe split node into GroupingNodes and Objects node, 
// where grouping nodes are for storing children only and ObjectNodes hold pointers to objects
class Scene {
public:	
	Node* m_selectedNode;

	Node* AttachObject(Object* object);
	void DrawSceneHierarchy();
	void UpdateScene();
	void RenderScene(RenderData* renderData);
	void UpdateSelectedNode();
private:
	Node rootNode;

};
