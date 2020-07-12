#include "BezierSurfaceC2.h"

BezierSurfaceC2::BezierSurfaceC2(std::vector<std::shared_ptr<Node>> patches, int wCount, int hCount, SurfaceWrapDirection wrapDirection): 
	BezierSurfaceC0(patches, wCount, hCount, wrapDirection)
{

}

BezierSurfaceC2::~BezierSurfaceC2()
{
}

BezierPatch* BezierSurfaceC2::GetPatchAtParameter(float& u, float& v)
{
	// u - width
	// v - height 

	// determine the W and H of a given patch and get the point from this patch	
	assert(ParamsInsideBounds(u, v));
	GetWrappedParams(u, v);

	int w = (int)(u * (float)m_patchW);
	int h = (int)(v * (float)m_patchH);

	if (w == m_patchW)
		w--;
	if (h == m_patchH)
		h--;

	float uStep = 1.f / (float)m_patchW;
	float vStep = 1.f / (float)m_patchH;

	float newU = (u - (float)w / (float)m_patchW) * m_patchW;
	float newV = (v - (float)h / (float)m_patchH) * m_patchH;

	auto node = GetPatch(w, h).lock();

	u = 1.f - newU;
	v = newV;

	return (BezierPatch*)(node->m_object).get();
	
}
