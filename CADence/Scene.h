#pragma once
#include <vector>
#include "cadStructures.h"

struct Node
{
public:
	std::vector<Node> children;
	Object* object;
	void AttachChild(Object* object);
	void DrawHierarchyNode();
	void Update();
	void Render();
private:
	bool guiParamsChanged;
};

class Scene {
public:	
	void AttachObject(Object* object);
	void DrawSceneHierarchy();
	void UpdateScene();
	void RenderScene();

private:
	Node rootNode;

};
