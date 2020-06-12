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
		if (it->m_refered == existingRef)
		{						
			it->m_refered = newRef;
			// Add a parent ref for (this) to newRef
			auto refs = newRef->GetReferences();
			auto childPar = m_owner;
			refs.AddParentRef(childPar);			
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
	if (auto child = reference)
	{
		AddRef(reference);
		child->GetReferences().AddParentRef(m_owner);
	}	
}

void ObjectReferences::LinkParentRef(ObjectRef parentReference)
{
	if (auto parent = parentReference)
	{
		AddParentRef(parentReference);
		parent->GetReferences().AddRef(m_owner);
	}
}

void ObjectReferences::UnlinkRef(ObjectRef reference)
{
	RemoveRef(reference);
	auto thisRef = m_owner->m_nodePtr;
	if (auto child = reference)
	{
		child->GetReferences().RemoveParentRef(m_owner);
	}
}

void ObjectReferences::UnlinkParentRef(ObjectRef reference)
{
	RemoveParentRef(reference);
	if (auto parent = reference)
	{
		parent->GetReferences().RemoveRef(m_owner);
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
	for (auto it = m_children.begin(); it != m_children.end();)
	{
		
		if (it->m_refered == reference)
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

// Remove all parent references to the given object
void ObjectReferences::RemoveParentRef(ObjectRef reference)
{	
	for (auto it = m_parents.begin(); it != m_parents.end();)
	{
		if (it->m_refered == reference)
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

Reference::Reference(ObjectRef referedObj)
{
	m_refered = referedObj;
}
