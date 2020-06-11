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
				auto refs = object->m_object->GetReferences();
				refs.AddParentRef(childPar);
			}
		}
	}
}

std::vector<Reference> ObjectReferences::GetAllRef()
{
	return m_children;
}

std::vector<Reference> ObjectReferences::GetAllRefParents()
{
	return m_parents;
}

void ObjectReferences::LinkRef(ObjectRef reference)
{
	if (auto child = reference.lock())
	{
		auto thisRef = m_owner->m_nodePtr;
		AddRef(reference);
		child->m_object->GetReferences().AddParentRef(thisRef);
	}	
}

void ObjectReferences::LinkParentRef(ObjectRef parentReference)
{
	if (auto parent = parentReference.lock())
	{
		auto thisRef = m_owner->m_nodePtr;
		AddParentRef(parentReference);
		parent->m_object->GetReferences().AddRef(thisRef);
	}
}

void ObjectReferences::UnlinkRef(ObjectRef reference)
{
	RemoveRef(reference);
	auto thisRef = m_owner->m_nodePtr;
	if (auto child = reference.lock())
	{
		child->m_object->GetReferences().RemoveParentRef(thisRef);
	}
}

void ObjectReferences::UnlinkParentRef(ObjectRef reference)
{
	RemoveParentRef(reference);
	auto thisRef = m_owner->m_nodePtr;
	if (auto parent = reference.lock())
	{
		parent->m_object->GetReferences().RemoveRef(thisRef);
	}
}

void ObjectReferences::AddRef(ObjectRef reference)
{
	m_children.push_back(Reference(reference));
}

void ObjectReferences::AddParentRef(ObjectRef reference)
{
	m_parents.push_back(Reference(reference));
}

// Remove all child references to the given object
void ObjectReferences::RemoveRef(ObjectRef reference)
{
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

Reference::Reference(ObjectRef referedObj)
{
	m_refered = referedObj;
}
