#include "mathUtils.h"


// https://en.wikibooks.org/wiki/Algorithm_Implementation/Linear_Algebra/Tridiagonal_matrix_algorithm
//| b0 c0 0 || x0 |  |d0|
//|a1 b1 c1 || x1 |= |d1|
//|0  a2 b2 || x2 |  |d2|
std::vector<float> SolveTridiagMatrix(std::vector<float> lowerDiag, std::vector<float> diag, std::vector<float> upperDiag, std::vector<float> vector)
{

	std::vector<float> a = lowerDiag;
	std::vector<float> b = diag;
	std::vector<float> c = upperDiag;
	std::vector<float> d = vector;
	std::vector<float> x = d;

	/*std::vector<float> a = { 0, -1, -1, -1 };
	std::vector<float> b = { 4,  4,  4,  4 };
	std::vector<float> c = { -1, -1, -1,  0 };
	std::vector<float> d = { 5,  5, 10, 23 };*/
	// res = [2 ,3  ,5 ,7]
	int X = d.size();

	c[0] = c[0] / b[0];
	x[0] = x[0] / b[0];

	/* loop from 1 to X - 1 inclusive, performing the forward sweep */
	for (size_t ix = 1; ix < X; ix++) {
		const float m = 1.0f / (b[ix] - a[ix] * c[ix - 1]);
		c[ix] = c[ix] * m;
		x[ix] = (x[ix] - a[ix] * x[ix - 1]) * m;
	}

	/* loop from X - 2 to 0 inclusive (safely testing loop condition for an unsigned integer), to perform the back substitution */
	for (size_t ix = X - 2; ix > 0; ix--)
		x[ix] -= c[ix] * x[ix + 1];
	x[0] -= c[0] * x[1];

	return x;
}

DirectX::XMFLOAT3 XMF3SUM(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2)
{
	return DirectX::XMFLOAT3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

DirectX::XMFLOAT3 XMF3SUB(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2)
{
	return DirectX::XMFLOAT3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}


DirectX::XMFLOAT3 XMF3TimesFloat(DirectX::XMFLOAT3 vec, float val)
{
	DirectX::XMFLOAT3 res = DirectX::XMFLOAT3(
		vec.x * val,
		vec.y * val,
		vec.z * val);

	return res;
}

// t * p0 + (1-t) * p1
Transform WeightedTransformAverage(Transform p0, Transform p1, float t)
{
	return Transform(
		WeightedXMFloat3Average(p0.GetPosition(), p1.GetPosition(), t),
		WeightedXMFloat3Average(p0.GetRotation(), p1.GetRotation(), t),
		WeightedXMFloat3Average(p0.GetScale(), p1.GetScale(), t));
}

DirectX::XMFLOAT3 WeightedXMFloat3Average(DirectX::XMFLOAT3 f1, DirectX::XMFLOAT3 f2, float t)
{
	// TODO : REMOVE
	return XMF3SUM(XMF3TimesFloat(f1, t), XMF3TimesFloat(f2, 1.0f - t));
}

float GetDistanceBetweenPoints(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2)
{
	return sqrtf((p1.x-p2.x)* (p1.x - p2.x) + (p1.y - p2.y)* (p1.y - p2.y)+ (p1.z - p2.z)* (p1.z - p2.z));
}

float Dot(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2)
{
	return p1.x * p2.x + p1.y * p2.y + p2.z * p2.z;
}

DirectX::XMFLOAT3 Cross(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2)
{
	float x = p1.y * p2.z - p1.z * p2.y;
	float y = p1.z * p2.x - p1.x * p2.z;
	float z = p1.x * p2.y - p1.y * p2.x;
	return DirectX::XMFLOAT3(x,y,z);
}

DirectX::XMFLOAT3& operator-(XMFLOAT3 a, XMFLOAT3 b)
{
	return a + (-1.f) * b;
}

DirectX::XMFLOAT3& operator+(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2)
{
	DirectX::XMFLOAT3 res = DirectX::XMFLOAT3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
	return res;
}

DirectX::XMFLOAT3& operator*(DirectX::XMFLOAT3 val, float t)
{
	DirectX::XMFLOAT3 res = DirectX::XMFLOAT3(
		val.x * t,
		val.y * t,
		val.z * t);
	
	return res;
}

DirectX::XMFLOAT3& operator/(XMFLOAT3 val, float t)
{
	return val * (1.f / t);
}

DirectX::XMFLOAT3& operator*(float t, XMFLOAT3 val)
{
	return val * t;
}
