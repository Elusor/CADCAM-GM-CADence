#include "MeshObject.h"

void MeshObject::RenderObject()
{
	m_renderer.RenderMesh(this);
}
