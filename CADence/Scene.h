#pragma once
#include <vector>
#include "cadStructures.h"
#include "Node.h"
#include "ObjectFactory.h"
// Maybe split node into GroupingNodes and Objects node, 
// where grouping nodes are for storing children only and ObjectNodes hold pointers to objects
class Scene {
public:	
	Scene();
	std::unique_ptr<ObjectFactory> m_objectFactory;
	std::unique_ptr<SpawnMarker> m_spawnMarker;
	std::weak_ptr<Node> m_selectedNode;
	std::vector<std::shared_ptr<Node>> m_nodes;

	void AttachObject(std::unique_ptr<Object>& object);
	void RemoveObject(std::unique_ptr<Object>& object);
	void DrawSceneHierarchy();

	void UpdateScene();
	void UpdateSelectedNode();
	void RenderScene(std::unique_ptr<RenderData> & renderData);

private:
	void ClearScene();
	void DrawScenePopupMenu();
	void DrawNodePopupMenu(const std::shared_ptr<Node> node);
};
