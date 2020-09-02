#include "BezierSurfaceC2.h"

BezierSurfaceC2::BezierSurfaceC2(std::vector<std::shared_ptr<Node>> patches, int wCount, int hCount, SurfaceWrapDirection wrapDirection): 
	BezierSurfaceC0(patches, wCount, hCount, wrapDirection)
{

}

BezierSurfaceC2::~BezierSurfaceC2()
{
}