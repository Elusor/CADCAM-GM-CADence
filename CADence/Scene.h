#pragma once
#include <vector>
#include "objects.h"
#include "Node.h"
#include "ObjectFactory.h"
#include "CurveVisualizer.h"

// Maybe split node into GroupingNodes and Objects node, 
// where grouping nodes are for storing children only and ObjectNodes hold pointers to objects
class Scene {
public:	
	std::unique_ptr<SceneGrid> m_grid;
	std::unique_ptr<ObjectFactory> m_objectFactory;
	std::unique_ptr<SpawnMarker> m_spawnMarker;
	std::unique_ptr<Object> m_middleMarker;
	std::unique_ptr<CurveVisualizer> m_curveVisualizer;
	std::vector<std::weak_ptr<Node>> m_selectedNodes;
	std::vector<std::shared_ptr<Node>> m_nodes;

	Scene(GuiManager* guiManager, RenderState* renderState);
	~Scene();

	void AttachObject(std::shared_ptr<Node> obj);
	void RemoveObject(std::unique_ptr<Object>& object);
	void DrawSceneHierarchy(bool filtered);

	void Select(std::weak_ptr<Node> obj, bool unique = true);
	void Select(std::vector<std::weak_ptr<Node>> obj, bool unique = true);
	void SelectionChanged(Node& node);
	void ClearSelection();

	void ClearModifiedTag();
	void UpdateScene();
	void LateUpdate();
	void UpdateSelectedNode();
	void RenderScene(std::unique_ptr<RenderState> & renderState);
	void ClearScene();
	
private:
	int m_sizeU;
	int m_sizeV;
	int m_sizeX;
	int m_sizeY;
	bool m_altState;
	bool m_altDir;

	void DrawScenePopupMenu();
	void DrawNodePopupMenu(const std::shared_ptr<Node> node);
	void RenderMiddleMarker(std::unique_ptr<RenderState>& renderState);
	void DrawBezierPatchModal();
	void DrawBezierSurfaceModal();
	void DrawBezierSurfaceC2Modal();
};
