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

float Mul(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2);
DirectX::XMFLOAT3 Mul(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3X3 p2);
DirectX::XMFLOAT3 Mul(DirectX::XMFLOAT3X3 p1, DirectX::XMFLOAT3 p2);

float Mul(DirectX::XMFLOAT4 p1, DirectX::XMFLOAT4 p2);
DirectX::XMFLOAT4 Mul(DirectX::XMFLOAT4 p1, DirectX::XMFLOAT4X4 p2);
DirectX::XMFLOAT4 Mul(DirectX::XMFLOAT4X4 p1, DirectX::XMFLOAT4 p2);

float Dot(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2);
DirectX::XMFLOAT3 Cross(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2);

float Dot(DirectX::XMFLOAT4 p1, DirectX::XMFLOAT4 p2);
//DirectX::XMFLOAT3 Cross(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2);

DirectX::XMFLOAT3 operator-(XMFLOAT3 a, XMFLOAT3 b);
DirectX::XMFLOAT3 operator+(XMFLOAT3 a, XMFLOAT3 b);
DirectX::XMFLOAT3 operator*(XMFLOAT3 val, float t);
DirectX::XMFLOAT3 operator/(XMFLOAT3 val, float t);
DirectX::XMFLOAT3 operator*(float t, XMFLOAT3 val);

DirectX::XMFLOAT4 operator+(XMFLOAT4 a, XMFLOAT4 b);
DirectX::XMFLOAT4 operator-(XMFLOAT4 a, XMFLOAT4 b);
DirectX::XMFLOAT4 operator*(XMFLOAT4 val, float t);
DirectX::XMFLOAT4 operator/(XMFLOAT4 val, float t);
DirectX::XMFLOAT4 operator*(float t, XMFLOAT4 val);

void SetNthFieldValue(DirectX::XMFLOAT4& vector, int index, float value);

float GetNthFieldValue(DirectX::XMFLOAT4 vector, int index);