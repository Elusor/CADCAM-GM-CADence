#pragma once
#include "Point.h"
#include "MeshObject.h"
class Node; 
class Scene;

struct BezierCurve : MeshObject
{
public:
	// TODO MG maybe just change objects to sharedptr?
	explicit BezierCurve(std::vector<std::weak_ptr<Node>> initialControlPoints);

	std::vector<std::weak_ptr<Node>> m_controlPoints;

	// Translation overrides 
	void SetTransform(Transform transform) override;	
	void SetPosition(DirectX::XMFLOAT3 position) override;
	void SetRotation(DirectX::XMFLOAT3 rotation) override;
	void SetScale(DirectX::XMFLOAT3 scale) override;
	void Translate(DirectX::XMFLOAT3 position) override;
	void Rotate(DirectX::XMFLOAT3 rotation) override;
	void Scale(DirectX::XMFLOAT3 scale) override;

	// Rendering and updates
	void RenderObjectSpecificContextOptions(Scene& scene) override;
	void RenderObject(std::unique_ptr<RenderState>& renderState) override;
	void UpdateObject() override;
	bool CreateParamsGui() override;

	// Child managment
	void AttachChild(std::weak_ptr<Node> controlPoint);
	void RemoveChild(std::weak_ptr<Node> controlPoint);	
	bool IsChild(std::weak_ptr<Node> point);	

protected:
	int m_adaptiveRenderingSamples;
	MeshDescription m_PolygonDesc;
	bool m_renderPolygon;
	void RenderPolygon(std::unique_ptr<RenderState>& renderState);
	void RemoveExpiredChildren();
};