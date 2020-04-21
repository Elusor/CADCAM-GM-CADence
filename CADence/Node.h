#pragma once
#include <vector>
#include <memory>
#include "objects.h"
class Scene;

// Represents a hierarchy node used to represent objects in the scene
class Node
{
public:
	std::unique_ptr<Object> m_object;
	bool m_isSelected = false;	
	bool m_isRenamed = false;
	bool m_isVirtual = false;

	virtual void DrawNodeGUI(Scene& scene);
	std::string GetLabel();
	void Update();
	void LateUpdate();
	void Render(std::unique_ptr<RenderState> & renderData);		
	
	// Methods that coul be further used for non flat hierarchy 
	virtual std::vector<std::weak_ptr<Node>> GetSelectedChildren();
	virtual std::vector<std::weak_ptr<Node>> GetChildren();
	virtual void ClearChildrenSelection();

protected:
	void DrawRenameGUI();
};
