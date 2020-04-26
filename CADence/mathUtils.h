#pragma once
#include "Transform.h"
#include "objects.h"
#include <memory>
#include <vector>
std::vector<float> SolveTridiagMatrix(std::vector<float> lowerDiag, std::vector<float> diag, std::vector<float> upperDiag, std::vector<float> vector);
DirectX::XMFLOAT3 XMF3SUM(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2);
DirectX::XMFLOAT3 XMF3SUB(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2);
DirectX::XMFLOAT3 XMF3TimesFloat(DirectX::XMFLOAT3 vec, float val);
Transform WeightedTransformAverage(Transform p0, Transform p1, float t);
DirectX::XMFLOAT3 WeightedXMFloat3Average(DirectX::XMFLOAT3 f1, DirectX::XMFLOAT3 f2, float t);
float GetDistanceBetweenPoints(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2);