#pragma once

class BezierPatch;
enum BoundaryDirection;
struct HoleData {
	bool isValid = false;
	BezierPatch* p1;
	BezierPatch* p2;
	BezierPatch* p3;
	BoundaryDirection p1Edge;
	BoundaryDirection p2Edge;
	BoundaryDirection p3Edge;
};

static class HoleDetector {
public:
	static HoleData DetectHole(BezierPatch* p1, BezierPatch* p2, BezierPatch* p3);
private:

};