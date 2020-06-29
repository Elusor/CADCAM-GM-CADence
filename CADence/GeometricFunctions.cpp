#include "GeometricFunctions.h"

// Finds the row with largest value in a given column below the diagonal
int FindLargestRow(DirectX::XMFLOAT4X4 matrix, int column)
{
	float best = abs(matrix.m[column][column]);
	int row = column;
	for (int r = column + 1; r < 4; r++)
	{
		if (abs(matrix.m[r][column]) > best)
		{
			best = abs(matrix.m[r][column]);
			row = r;
		}
	}

	return row;
}

void SetNthFieldValue(DirectX::XMFLOAT4& vector, int index, float value) {
	switch (index)
	{
	case 0:
		vector.x = value;
		break;
	case 1:
		vector.y = value;
		break;
	case 2:
		vector.z = value;
		break;
	case 3:
		vector.w = value;
		break;
	}
}

float GetNthFieldValue(DirectX::XMFLOAT4 vector, int index) {

	float res = 0.0f;
	switch (index)
	{
	case 0:
		res = vector.x;
		break;
	case 1:
		res = vector.y;
		break;
	case 2:
		res = vector.z;
		break;
	case 3:
		res = vector.w;
		break;
	}
	return res;
}

void MultiplyRow(DirectX::XMFLOAT4X4& matrix, DirectX::XMFLOAT4& bVector, int alteredRow, float multiplier = 1.0f)
{
	float bVal = GetNthFieldValue(bVector, alteredRow);
	float bModified = bVal * multiplier;

	DirectX::XMFLOAT4 rowVals;
	rowVals.x = matrix.m[alteredRow][0];
	rowVals.y = matrix.m[alteredRow][1];
	rowVals.z = matrix.m[alteredRow][2];
	rowVals.w = matrix.m[alteredRow][3];

	DirectX::XMFLOAT4 modifiedRowVals;
	modifiedRowVals.x = rowVals.x * multiplier;
	modifiedRowVals.y = rowVals.y * multiplier;
	modifiedRowVals.z = rowVals.z * multiplier;
	modifiedRowVals.w = rowVals.w * multiplier;

	matrix.m[alteredRow][0] = modifiedRowVals.x;
	matrix.m[alteredRow][1] = modifiedRowVals.y;
	matrix.m[alteredRow][2] = modifiedRowVals.z;
	matrix.m[alteredRow][3] = modifiedRowVals.w;
	SetNthFieldValue(bVector, alteredRow, bModified);
}

// Performs alteredRow -= valuesRow * multiplier
void SubstractRows(DirectX::XMFLOAT4X4& matrix, DirectX::XMFLOAT4& bVector, int alteredRow, int valuesRow, float multiplier = 1.0f)
{
	float bVal = GetNthFieldValue(bVector, alteredRow);
	float bModified = bVal - GetNthFieldValue(bVector, valuesRow) * multiplier;

	DirectX::XMFLOAT4 rowVals;
	rowVals.x = matrix.m[alteredRow][0];
	rowVals.y = matrix.m[alteredRow][1];
	rowVals.z = matrix.m[alteredRow][2];
	rowVals.w = matrix.m[alteredRow][3];

	DirectX::XMFLOAT4 modifiedRowVals;
	modifiedRowVals.x = rowVals.x - matrix.m[valuesRow][0] * multiplier;
	modifiedRowVals.y = rowVals.y - matrix.m[valuesRow][1] * multiplier;
	modifiedRowVals.z = rowVals.z - matrix.m[valuesRow][2] * multiplier;
	modifiedRowVals.w = rowVals.w - matrix.m[valuesRow][3] * multiplier;

	matrix.m[alteredRow][0] = modifiedRowVals.x;
	matrix.m[alteredRow][1] = modifiedRowVals.y;
	matrix.m[alteredRow][2] = modifiedRowVals.z;
	matrix.m[alteredRow][3] = modifiedRowVals.w;
	SetNthFieldValue(bVector, alteredRow, bModified);
}

void SwapRows(DirectX::XMFLOAT4X4& matrix, DirectX::XMFLOAT4& bVector, int row1, int row2)
{
	if (row1 == row2) return;

	DirectX::XMFLOAT4 tmp;
	float bTmp;

	// Stash values from row1 
	tmp.x = matrix.m[row1][0];
	tmp.y = matrix.m[row1][1];
	tmp.z = matrix.m[row1][2];
	tmp.w = matrix.m[row1][3];
	bTmp = GetNthFieldValue(bVector, row1);

	// Copy values from row2 to row1
	matrix.m[row1][0] = matrix.m[row2][0];
	matrix.m[row1][1] = matrix.m[row2][1];
	matrix.m[row1][2] = matrix.m[row2][2];
	matrix.m[row1][3] = matrix.m[row2][3];
	SetNthFieldValue(bVector, row1, GetNthFieldValue(bVector, row2));

	// Copy stashed values to row2
	matrix.m[row2][0] = tmp.x;
	matrix.m[row2][1] = tmp.y;
	matrix.m[row2][2] = tmp.z;
	matrix.m[row2][3] = tmp.w;
	SetNthFieldValue(bVector, row2, bTmp);
}

DirectX::XMFLOAT4 Geom::SolveGEPP(DirectX::XMFLOAT4X4 A, DirectX::XMFLOAT4 bVec)
{
	// Reduce elements below the diagonal
	for (int col = 0; col < 4; col++)
	{
		// Search for largest absolute value and swap it with the current row
		int rowIdx = FindLargestRow(A, col);
		SwapRows(A, bVec, col, rowIdx);
		assert(A.m[col][col] != 0);
		//Swap C an B rows and also b vector rows
		for (int r = col+1; r < 4; r++)
		{
			// Zero all values below the diagonal
			float alpha = A.m[r][col] / A.m[col][col];
			SubstractRows(A, bVec, r, col, alpha);
		}		

	}

	// Reduce the elements abovethe diagonal
	for (int col = 3; col >= 0; col--)
	{
		for (int row = col - 1; row >= 0; row--)
		{
			float alpha = A.m[row][col] / A.m[col][col];
			SubstractRows(A, bVec, row, col, alpha);
		}

		//Set the element on the diagonal to 1 
		float normalizer = 1.f / A.m[col][col];
		MultiplyRow(A, bVec, col, normalizer);
	}

	return bVec;
}