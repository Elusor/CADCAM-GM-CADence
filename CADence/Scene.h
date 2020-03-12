#pragma once
#include <vector>
#include "cadStructures.h"
// Maybe split node into GroupingNodes and Objects node, 
// where grouping nodes are for storing children only and ObjectNodes hold pointers to objects
struct Node
{
public:
	std::vector<Node*> children;
	Object* object;
	Node* AttachChild(Object* object);
	void DrawHierarchyNode();
	void Update();
	void Render();
private:
	bool isDirty;
};

class Scene {
public:	
	Node* m_selectedNode;

	Node* AttachObject(Object* object);
	void DrawSceneHierarchy();
	void UpdateScene();
	void RenderScene();
private:
	Node rootNode;

};
