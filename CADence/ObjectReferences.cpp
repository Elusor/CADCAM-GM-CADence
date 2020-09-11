#include "ObjectReferences.h"
#include "Object.h"

ObjectReferences::ObjectReferences(Object* object): m_owner(object)
{
}

// Substitutes all references to existing ref with new ref and sets parent refs in newRef
void ObjectReferences::SubstituteReference(ObjectRef existingRef, ObjectRef newRef)
{
	for (auto it = m_children.begin(); it != m_children.end(); it++)
	{
		if (it->m_refered.lock() == existingRef.lock())
		{			
			if (auto object = newRef.lock())
			{
				it->m_refered = newRef;
				auto childPar = m_owner->m_nodePtr;
				// Add a parent ref for (this) to newRef
				//std::find()
				object->m_object->GetReferences().AddParentRef(childPar);
			}
		}
	}
}

std::vector<ObjectRef> ObjectReferences::GetRefObjects()
{
	std::vector<ObjectRef> objects;
	for (int i = 0; i < m_children.size(); i++)
	{
		objects.push_back(m_children[i].m_refered);
	}

	return objects;
}

std::vector<Reference>& ObjectReferences::GetAllRef()
{
	return m_children;
}

std::vector<Reference>& ObjectReferences::GetAllRefParents()
{
	return m_parents;
}

void ObjectReferences::LinkRef(ObjectRef reference)
{
	assert(!reference.expired());
	if (auto child = reference.lock())
	{
		auto thisRef = m_owner->m_nodePtr;
		AddRef(reference);
		m_owner->RefUse();
		child->m_object->GetReferences().AddParentRef(thisRef);
		child->m_object->RefUse();
	}	
}

void ObjectReferences::LinkParentRef(ObjectRef parentReference)
{
	assert(!parentReference.expired());
	if (auto parent = parentReference.lock())
	{
		auto thisRef = m_owner->m_nodePtr;
		AddParentRef(parentReference);
		parent->m_object->GetReferences().AddRef(thisRef);
	}
}

void ObjectReferences::UnlinkRef(ObjectRef reference)
{
	assert(!reference.expired());
	RemoveRef(reference);
	auto thisRef = m_owner->m_nodePtr;
	m_owner->RefRelease();
	if (auto child = reference.lock())
	{
		child->m_object->GetReferences().RemoveParentRef(thisRef);
		child->m_object->RefRelease();
	}
}

void ObjectReferences::UnlinkParentRef(ObjectRef reference)
{
	assert(!reference.expired());
	RemoveParentRef(reference);
	auto thisRef = m_owner->m_nodePtr;
	if (auto parent = reference.lock())
	{
		parent->m_object->GetReferences().RemoveRef(thisRef);
	}
}

void ObjectReferences::UnlinkAll()
{
	while (m_children.size() > 0)
	{
		UnlinkRef(m_children[m_children.size() - 1].m_refered);
	}

	while (m_parents.size() > 0)
	{
		UnlinkParentRef(m_parents[m_parents.size() - 1].m_refered);
	}
}

void ObjectReferences::AddRef(ObjectRef reference)
{
	assert(!reference.expired());
	m_children.push_back(Reference(reference));
}

void ObjectReferences::AddParentRef(ObjectRef reference)
{
	assert(!reference.expired());
	m_parents.push_back(Reference(reference));
}

// Remove all child references to the given object
void ObjectReferences::RemoveRef(ObjectRef reference)
{
	assert(!reference.expired());
	if (auto obj = reference.lock())
	{
		for (auto it = m_children.begin(); it != m_children.end();)
		{
		
			if (it->m_refered.lock() == obj)
			{
				// call Remove parent ref on this child
				it = m_children.erase(it);
				//auto childPar = m_owner->m_parent;				
			}
			else
			{
				it++;
			}
		}		
	}
}

// Remove all parent references to the given object
void ObjectReferences::RemoveParentRef(ObjectRef reference)
{
	assert(!reference.expired());
	if (auto obj = reference.lock())
	{
		for (auto it = m_parents.begin(); it != m_parents.end();)
		{
			if (it->m_refered.lock() == obj)
			{
				// call Remove child ref on this child
				it = m_parents.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}

bool ObjectReferences::RemovedExpiredReferences()
{
	bool anyRemoved = false;

	auto it = m_children.begin();
	while (it != m_children.end())
	{
		if (auto pt = it->m_refered.lock())
		{
			it++;
		}
		else {
			it = m_children.erase(it);
			anyRemoved = true;
		}
	}

	auto it2 = m_parents.begin();
	while (it2 != m_parents.end())
	{
		if (auto pt = it2->m_refered.lock())
		{
			it2++;
		}
		else {
			it2 = m_parents.erase(it2);
			anyRemoved = true;
		}
	}

	return anyRemoved;
}

Reference::Reference(ObjectRef referedObj)
{
	m_refered = referedObj;
}
