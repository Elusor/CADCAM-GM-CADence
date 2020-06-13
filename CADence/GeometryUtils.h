#pragma once
#include <vector>
#include <memory>
class Node;
class Transform;

struct DividedCurve {
	std::vector<DirectX::XMFLOAT3> begMid;
	std::vector<DirectX::XMFLOAT3> midEnd;
};

DividedCurve* DivideBernsteinCurve(std::vector<std::weak_ptr<Node>> bernsteinCurve, float point = 0.5f);