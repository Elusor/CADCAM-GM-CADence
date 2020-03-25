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

	virtual void DrawNodeGUI(Scene& scene);
	std::string GetLabel();
	void Update();
	void Render(std::unique_ptr<RenderState> & renderData);	

protected:
	void DrawRenameGUI();
};
