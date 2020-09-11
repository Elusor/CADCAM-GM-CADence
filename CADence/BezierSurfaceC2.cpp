#include "BezierSurfaceC2.h"

BezierSurfaceC2::BezierSurfaceC2(std::vector<std::shared_ptr<Node>> patches, int wCount, int hCount, SurfaceWrapDirection wrapDirection): 
	BezierSurfaceC0(patches, wCount, hCount, wrapDirection)
{
	for (int i = 0; i < patches.size(); i++)
	{
		patches[i]->m_object->RefUse();
	}
}

BezierSurfaceC2::~BezierSurfaceC2()
{
	// unmark all patches and remove them
	for (int i = 0; i < m_patches.size(); i++)
	{
		m_patches[i]->m_object->GetReferences().UnlinkAll();
	}
}