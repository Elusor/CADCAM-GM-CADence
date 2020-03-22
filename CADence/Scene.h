#pragma once
#include <vector>
#include "objects.h"
#include "Node.h"
#include "ObjectFactory.h"
using namespace std;
// Maybe split node into GroupingNodes and Objects node, 
// where grouping nodes are for storing children only and ObjectNodes hold pointers to objects
class Scene {
public:	
	Scene();
	unique_ptr<ObjectFactory> m_objectFactory;
	unique_ptr<SpawnMarker> m_spawnMarker;
	unique_ptr<Object> m_middleMarker;
	vector<weak_ptr<Node>> m_selectedNodes;
	vector<shared_ptr<Node>> m_nodes;

	void AttachObject(std::unique_ptr<Object>& object);
	void RemoveObject(std::unique_ptr<Object>& object);

	void DrawSceneHierarchy();
	void DrawSceneGUI();

	void UpdateScene();
	void UpdateSelectedNodes();
	void RenderScene();

private:

	void GetMiddleMarkerCoords();

	void ClearScene();
	void DrawScenePopupMenu();
	void DrawNodePopupMenu(const std::shared_ptr<Node> node);
};
