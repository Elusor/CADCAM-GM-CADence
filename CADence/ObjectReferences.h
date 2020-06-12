#pragma once
#include <xmemory>
#include "Node.h"

typedef Object* ObjectRef;
class Object; 

struct Reference {
	Reference(ObjectRef referedObj);
	ObjectRef m_refered;
	// ref type
};

// Represents ALL reference to existing NON-VIRTUAL objects on the scene
class ObjectReferences {
public:

	ObjectReferences(Object* object);

	void SubstituteReference(ObjectRef existingRef, ObjectRef newRef);

	std::vector<Reference> GetAllRef();
	std::vector<Reference> GetAllRefParents();

	// Link Unlink methods that execute Add ref for object and Add parent ref as well
	void LinkRef(ObjectRef reference);
	void LinkParentRef(ObjectRef reference);
	void UnlinkRef(ObjectRef reference);
	void UnlinkParentRef(ObjectRef reference);

	void AddRef(ObjectRef reference);
	void AddParentRef(ObjectRef reference);
	void RemoveRef(ObjectRef reference);
	void RemoveParentRef(ObjectRef reference);

private:
	Object* m_owner;
	std::vector<Reference> m_parents;
	std::vector<Reference> m_children;
};