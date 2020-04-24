#include "mathUtils.h"


// https://en.wikibooks.org/wiki/Algorithm_Implementation/Linear_Algebra/Tridiagonal_matrix_algorithm
//| b0 c0 0 || x0 |  |d0|
//|a1 b1 c1 || x1 |= |d1|
//|0  a2 b2 || x2 |  |d2|
std::vector<float> SolveTridiagMatrix(std::vector<float> lowerDiag, std::vector<float> diag, std::vector<float> upperDiag, std::vector<float> vector)
{

	int size = vector.size();

	std::vector<float> a = lowerDiag;
	std::vector<float> b = diag;
	std::vector<float> c = upperDiag;
	std::vector<float> d = vector;
	std::vector<float> res = d;
	/*
	// n is the number of unknowns

	|b0 c0 0 ||x0| |d0|
	|a1 b1 c1||x1|=|d1|
	|0  a2 b2||x2| |d2|

	1st iteration: b0x0 + c0x1 = d0 -> x0 + (c0/b0)x1 = d0/b0 ->

		x0 + g0x1 = r0               where g0 = c0/b0        , r0 = d0/b0

	2nd iteration:     | a1x0 + b1x1   + c1x2 = d1
		from 1st it.: -| a1x0 + a1g0x1        = a1r0
					-----------------------------
						  (b1 - a1g0)x1 + c1x2 = d1 - a1r0

		x1 + g1x2 = r1               where g1=c1/(b1 - a1g0) , r1 = (d1 - a1r0)/(b1 - a1g0)

	3rd iteration:      | a2x1 + b2x2   = d2
		from 2nd it. : -| a2x1 + a2g1x2 = a2r2
					   -----------------------
					   (b2 - a2g1)x2 = d2 - a2r2
		x2 = r2                      where                     r2 = (d2 - a2r2)/(b2 - a2g1)
	Finally we have a triangular matrix:
	|1  g0 0 ||x0| |r0|
	|0  1  g1||x1|=|r1|
	|0  0  1 ||x2| |r2|

	Condition: ||bi|| > ||ai|| + ||ci||

	in this version the c matrix reused instead of g
	and             the d matrix reused instead of r and x matrices to report results
	Written by Keivan Moradi, 2014
	*/
	int n = vector.size() - 1; // since we start from x0 (not x1)
	c[0] /= b[0];
	d[0] /= b[0];

	for (int i = 1; i < n; i++) {
		c[i] /= b[i] - a[i] * c[i - 1];
		d[i] = (d[i] - a[i] * d[i - 1]) / (b[i] - a[i] * c[i - 1]);
	}

	res[n] = d[n];

	for (int i = n - 1; i >= 0; i--)
	{
		res[i] = d[i] - c[i] * res[i + 1];
	}

	return res;
}

DirectX::XMFLOAT3 XMF3SUM(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2)
{
	return DirectX::XMFLOAT3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

DirectX::XMFLOAT3 XMF3SUB(DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2)
{
	return DirectX::XMFLOAT3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}


DirectX::XMFLOAT3 XMFloat3TimesFloat(DirectX::XMFLOAT3 vec, float val)
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
	return XMF3SUM(XMFloat3TimesFloat(f1, t), XMFloat3TimesFloat(f2, 1.0f - t));
}