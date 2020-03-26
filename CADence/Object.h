#pragma once
#include <string>
#include "Transform.h"
#include "renderState.h"

class Scene;
class Node; 

struct Object {
	std::string m_name = "";
	std::string m_defaultName = "";
	std::weak_ptr<Node> m_parent;
	
	Transform& GetTransform();
	virtual void SetTransform(Transform transform);
	virtual void Translate(DirectX::XMFLOAT3 position);
	virtual void Rotate(DirectX::XMFLOAT3 rotation);
	virtual void Scale(DirectX::XMFLOAT3 scale);

	virtual void SetPosition(DirectX::XMFLOAT3 position);
	virtual void SetRotation(DirectX::XMFLOAT3 rotation);
	virtual void SetScale(DirectX::XMFLOAT3 scale);

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();

	virtual void RenderObject(std::unique_ptr<RenderState>& renderState);
	virtual void RenderCoordinates(std::unique_ptr<RenderState>& renderState);
	virtual void RenderObjectSpecificContextOptions(Scene& scene);
	virtual void UpdateObject();
	virtual bool CreateParamsGui();

protected:
	Transform m_transform;
};