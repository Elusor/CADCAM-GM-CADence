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

float Mul(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2)
{
	return Dot(p1,p2);
}

DirectX::XMFLOAT3 Mul(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3X3 p2)
{
	DirectX::XMFLOAT3 col1, col2, col3;

	col1 = DirectX::XMFLOAT3(p2._11, p2._21, p2._31);
	col2 = DirectX::XMFLOAT3(p2._12, p2._22, p2._32);
	col3 = DirectX::XMFLOAT3(p2._13, p2._23, p2._33);

	float dot1, dot2, dot3;
	dot1 = Mul(p1, col1);
	dot2 = Mul(p1, col2);
	dot3 = Mul(p1, col3);

	return DirectX::XMFLOAT3(dot1, dot2, dot3);
}

DirectX::XMFLOAT3 Mul(DirectX::XMFLOAT3X3 p1, DirectX::XMFLOAT3 p2)
{
	DirectX::XMFLOAT3 row1, row2, row3;

	row1 = DirectX::XMFLOAT3(p1._11, p1._12, p1._13);
	row2 = DirectX::XMFLOAT3(p1._21, p1._22, p1._23);
	row3 = DirectX::XMFLOAT3(p1._31, p1._32, p1._33);

	float dot1, dot2, dot3;
	dot1 = Mul(row1, p2);
	dot2 = Mul(row2, p2);
	dot3 = Mul(row3, p2);

	return DirectX::XMFLOAT3(dot1, dot2, dot3);
}

float Mul(DirectX::XMFLOAT4 p1, DirectX::XMFLOAT4 p2)
{	
	return Dot(p1, p2);
}

DirectX::XMFLOAT4 Mul(DirectX::XMFLOAT4 p1, DirectX::XMFLOAT4X4 p2)
{
	DirectX::XMFLOAT4 col1, col2, col3, col4;
	
	col1 = DirectX::XMFLOAT4(p2._11, p2._21, p2._31, p2._41);
	col2 = DirectX::XMFLOAT4(p2._12, p2._22, p2._32, p2._42);
	col3 = DirectX::XMFLOAT4(p2._13, p2._23, p2._33, p2._43);
	col4 = DirectX::XMFLOAT4(p2._14, p2._24, p2._34, p2._44);

	float dot1, dot2, dot3, dot4;
	dot1 = Mul(p1, col1);
	dot2 = Mul(p1, col2);
	dot3 = Mul(p1, col3);
	dot4 = Mul(p1, col4);

	return DirectX::XMFLOAT4(dot1, dot2, dot3, dot4);
}

DirectX::XMFLOAT4 Mul(DirectX::XMFLOAT4X4 p1, DirectX::XMFLOAT4 p2)
{
	DirectX::XMFLOAT4 row1, row2, row3, row4;

	row1 = DirectX::XMFLOAT4(p1._11, p1._12, p1._13, p1._14);
	row2 = DirectX::XMFLOAT4(p1._21, p1._22, p1._23, p1._24);
	row3 = DirectX::XMFLOAT4(p1._31, p1._32, p1._33, p1._34);
	row4 = DirectX::XMFLOAT4(p1._41, p1._42, p1._43, p1._44);

	float dot1, dot2, dot3, dot4;
	dot1 = Mul(row1, p2);
	dot2 = Mul(row2, p2);
	dot3 = Mul(row3, p2);
	dot4 = Mul(row4, p2);

	return DirectX::XMFLOAT4(dot1, dot2, dot3, dot4);
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

float Dot(DirectX::XMFLOAT4 p1, DirectX::XMFLOAT4 p2)
{
	return p1.x * p2.x + p1.y * p2.y + p2.z * p2.z + p1.w * p2.w;
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

DirectX::XMFLOAT4& operator+(XMFLOAT4 a, XMFLOAT4 b)
{
	XMFLOAT4 res = XMFLOAT4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
	return res;
}

DirectX::XMFLOAT4& operator-(XMFLOAT4 a, XMFLOAT4 b)
{
	XMFLOAT4 res = a + (b * -1.f);
	return res;
}

DirectX::XMFLOAT4& operator*(XMFLOAT4 val, float t)
{
	XMFLOAT4 res = XMFLOAT4(val.x * t, val.y * t, val.z * t, val.w * t);
	return res;
}

DirectX::XMFLOAT4& operator/(XMFLOAT4 val, float t)
{
	return val * 1.f / t;
}

DirectX::XMFLOAT4& operator*(float t, XMFLOAT4 val)
{
	return val * t;
}
