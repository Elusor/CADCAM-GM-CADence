#pragma once
#include <string>
#include "Transform.h"
#include "renderState.h"
#include "ObjectReferences.h"

class Scene;
class Node; 

class Object {
public:
	Object();
	virtual ~Object() = default;
	int m_refCounter = 0;
	std::string m_name = "";
	std::string m_defaultName = "";
	std::weak_ptr<Node> m_nodePtr;

	std::string GetLabel();
	std::string GetIdentifier();

	void RefUse();
	void RefRelease();
	bool GetInUse();
	ObjectReferences& GetReferences();
	// Transform Wrappers
	Transform& GetTransform();
	virtual void SetTransform(Transform transform);
	virtual void Translate(DirectX::XMFLOAT3 position);
	virtual void Rotate(DirectX::XMFLOAT3 rotation);
	virtual void Scale(DirectX::XMFLOAT3 scale);

	virtual void SetPosition(DirectX::XMFLOAT3 position);
	virtual void SetRotation(DirectX::XMFLOAT3 rotation);
	virtual void SetScale(DirectX::XMFLOAT3 scale);
	virtual std::vector<std::weak_ptr<Node>> GetChildren();
	virtual std::vector<std::weak_ptr<Node>> GetDisplayChildren();

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();

	// Rendering and updates
	virtual void RenderObject(std::unique_ptr<RenderState>& renderState);
	virtual void RenderCoordinates(std::unique_ptr<RenderState>& renderState);
	virtual void RenderObjectSpecificContextOptions(Scene& scene);
	virtual void UpdateObject();
	virtual bool CreateParamsGui();

	virtual bool GetIsSelected();
	virtual void SetIsSelected(bool isSelected);
	virtual bool GetIsModified();	
	void SetModified(bool value);
protected:
	ObjectReferences m_references;
	Transform m_transform;
	bool m_modified = false;
	bool m_isSelected = false;

};