#pragma once
#include <vector>
#include "objects.h"
#include "Node.h"
#include "ObjectFactory.h"
// Maybe split node into GroupingNodes and Objects node, 
// where grouping nodes are for storing children only and ObjectNodes hold pointers to objects
class Scene {
public:	
	Scene();
	std::unique_ptr<ObjectFactory> m_objectFactory;
	std::unique_ptr<SpawnMarker> m_spawnMarker;
	std::unique_ptr<Object> m_middleMarker;
	//std::weak_ptr<Node> m_selectedNode;
	std::vector<std::weak_ptr<Node>> m_selectedNodes;
	std::vector<std::shared_ptr<Node>> m_nodes;

	void AttachObject(std::shared_ptr<Node>);
	void RemoveObject(std::unique_ptr<Object>& object);
	void DrawSceneHierarchy();

	void SelectionChanged(Node& node);

	void ClearModifiedTag();
	void UpdateScene();
	void UpdateSelectedNode();
	void RenderScene(std::unique_ptr<RenderState> & renderData);

private:
	void ClearScene();
	void DrawScenePopupMenu();
	void DrawNodePopupMenu(const std::shared_ptr<Node> node);
};
