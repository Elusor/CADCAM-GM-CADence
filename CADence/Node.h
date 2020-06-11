#pragma once
#include <vector>
#include <memory>
#include <string>
class Object;
class Scene;
class RenderState;

// Represents a hierarchy node used to represent objects in the scene
class Node
{
public:
	Node();
	Node(std::unique_ptr<Object> object, bool isVirtual);
	std::unique_ptr<Object> m_object;	
	
	void SetIsSelected(bool isSelected);
	bool GetIsSelected();
	void SetIsInactive(bool isInactive);
	bool GetIsInactive();
	void SetIsVirtual(bool isVirtual);
	bool GetIsVirtual();
	void Rename(std::string name);
	bool GetIsRenamed();
	std::string GetLabel();


	void Update();
	void LateUpdate();
	void Render(std::unique_ptr<RenderState> & renderState);		
	virtual void DrawNodeGUI(Scene& scene);

	// Methods that coul be further used for non flat hierarchy 
	virtual std::vector<std::weak_ptr<Node>> GetSelectedChildren();
	virtual std::vector<std::weak_ptr<Node>> GetChildren();
	virtual void ClearChildrenSelection();

protected:
	bool m_isInactive = false;
	bool m_isRenamed = false;
	bool m_isVirtual = false;

	void DrawRenameGUI();	
};
