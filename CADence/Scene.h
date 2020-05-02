#pragma once
#include <vector>
#include "objects.h"
#include "Node.h"
#include "ObjectFactory.h"
// Maybe split node into GroupingNodes and Objects node, 
// where grouping nodes are for storing children only and ObjectNodes hold pointers to objects
class Scene {
public:	

	std::unique_ptr<SceneGrid> m_grid;
	std::unique_ptr<ObjectFactory> m_objectFactory;
	std::unique_ptr<SpawnMarker> m_spawnMarker;
	std::unique_ptr<Object> m_middleMarker;
	std::vector<std::weak_ptr<Node>> m_selectedNodes;
	std::vector<std::shared_ptr<Node>> m_nodes;

	Scene();	

	void AttachObject(std::shared_ptr<Node>);
	void RemoveObject(std::unique_ptr<Object>& object);
	void DrawSceneHierarchy();

	void SelectionChanged(Node& node);

	void ClearModifiedTag();
	void UpdateScene();
	void LateUpdate();
	void UpdateSelectedNode();
	void RenderScene(std::unique_ptr<RenderState> & renderState);
	
private:
	

	void ClearScene();
	void DrawScenePopupMenu();
	void DrawNodePopupMenu(const std::shared_ptr<Node> node);
	void RenderMiddleMarker(std::unique_ptr<RenderState>& renderState);
};
