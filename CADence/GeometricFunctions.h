#include "mathUtils.h"

namespace Geom
{
	DirectX::XMFLOAT4 SolveGEPP(DirectX::XMFLOAT4X4 A, DirectX::XMFLOAT4 b);

	DirectX::XMFLOAT4 SolveGETP(DirectX::XMFLOAT4X4 A, DirectX::XMFLOAT4 b);
	// Calculate norm

	// Calculate Dot product

	// Calculate cross product

	template<int N>
	static void swap_rows(double mat[N][N + 1], int i, int j)
	{
		for (auto k = 0; k < N + 1; k++)
		{
			auto tmp = mat[i][k];
			mat[i][k] = mat[j][k];
			mat[j][k] = tmp;
		}
	}

	template<int N>
	static bool echelon_form(double mat[N][N + 1])
	{
		for (auto j = 0; j < N; j++)
		{
			auto i_max = j;
			auto val_max = mat[i_max][j];

			for (auto i = i_max + 1; i < N; i++)
			{
				if (abs(mat[i][j]) > abs(val_max))
				{
					i_max = i;
					val_max = mat[i][j];
				}
			}

			if (val_max == 0.0f) return false;

			if (i_max != j) swap_rows<N>(mat, i_max, j);

			for (auto i = j + 1; i < N; i++)
			{
				auto f = mat[i][j] / mat[j][j];

				for (auto k = j; k < N + 1; k++)
				{
					mat[i][k] -= mat[j][k] * f;
				}
			}
		}
	}

	template<int N>
	static void back_substitution(double mat[N][N + 1], double result[N])
	{
		for (auto i = N - 1; i >= 0; i--)
		{
			result[i] = mat[i][N];

			for (auto j = i + 1; j < N; j++)
			{
				result[i] -= mat[i][j] * result[j];
			}

			result[i] /= mat[i][i];
		}
	}

	template<int N>
	bool gaussianElimination(double mat[N][N + 1], double result[N])
	{
		auto done = echelon_form<N>(mat);
		if (!done) return false;

		back_substitution<N>(mat, result);

		return true;
	}
}