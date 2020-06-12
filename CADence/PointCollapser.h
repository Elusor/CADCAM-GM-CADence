#pragma once
#include <memory>
#include "Object.h"

class Scene;
class ObjectFactory;

class PointCollapser {
public:
	PointCollapser(Scene* scene);

	void Collapse(std::weak_ptr<Node> p1, std::weak_ptr<Node> p2);
private:
	Scene* m_scene;
	ObjectFactory* m_factory;
};