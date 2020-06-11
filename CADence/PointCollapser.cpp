#include "PointCollapser.h"
#include "Node.h"
#include "ObjectReferences.h"
#include "ObjectFactory.h"
#include "Scene.h"


PointCollapser::PointCollapser(Scene* scene, ObjectFactory* factory)
{
	m_scene = scene;
	m_factory = factory;
}

void PointCollapser::Collapse(std::weak_ptr<Node> p1, std::weak_ptr<Node> p2)
{
	auto point1 = p1.lock();
	auto point2 = p2.lock();

	auto parents1 = point1->m_object->GetReferences().GetAllRefParents();
	auto parents2 = point2->m_object->GetReferences().GetAllRefParents();

	// Create p3
	std::shared_ptr<Node> p3 = m_factory->CreatePoint();
	m_scene->AttachObject(p3);	

	for (auto it = parents1.begin(); it != parents1.end(); it++)
	{
		auto parentObject = it->m_refered.lock()->m_object.get();
		parentObject->GetReferences().SubstituteReference(p1, p3);
	}

	for (auto it = parents2.begin(); it != parents2.end(); it++)
	{
		auto parentObject = it->m_refered.lock()->m_object.get();
		parentObject->GetReferences().SubstituteReference(p2, p3);
	}

	// REMOVE p1 and p2 FROM SCENE
	m_scene->RemoveObject(point1->m_object);
	m_scene->RemoveObject(point2->m_object);
}

// Maybe just create the reference structure and stick it in Object class
// Then we could easily access all parents and just modify the control points in curves to a vector of these references
// We could modify patch to have a 16-ref vector filled with trash at the beggining
