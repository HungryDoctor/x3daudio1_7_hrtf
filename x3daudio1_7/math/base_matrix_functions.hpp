#pragma once

#include "base_matrix.hpp"

namespace math
{
	template <typename T, size_t COLS, size_t ROWS>
	base_matrix<T, COLS - 1, ROWS - 1> inline submatrix(const base_matrix<T, COLS, ROWS> & mat, size_t column, size_t row)
	{
		base_matrix<T, COLS - 1, ROWS - 1> res;
		for (size_t i = 0; i < row; i++)
		{
			res[i] = math::subvector(mat[i], column);
		}

		for (size_t i = row; i < ROWS - 1; i++)
		{
			res[i] = math::subvector(mat[i + 1], column);
		}

		return res;
	}

	template <typename T, size_t COLS, size_t ROWS, size_t EXCLUDE_COL, size_t EXCLUDE_ROW>
	base_matrix<T, COLS - 1, ROWS - 1> inline submatrix(const base_matrix<T, COLS, ROWS> & mat)
	{
		static_assert(EXCLUDE_COL >= 0, "EXCLUDE_COL can't be negative");
		static_assert(EXCLUDE_ROW >= 0, "EXCLUDE_ROW can't be negative");
		static_assert(EXCLUDE_COL < COLS, "EXCLUDE_COL must be less than COLS");
		static_assert(EXCLUDE_ROW < ROWS, "EXCLUDE_ROW must be less than ROWS");

		base_matrix<T, COLS - 1, ROWS - 1> res;
		for (size_t i = 0; i < EXCLUDE_ROW; i++)
		{
			res[i] = math::subvector<T, COLS, EXCLUDE_COL>(mat[i]);
		}

		for (size_t i = EXCLUDE_ROW; i < ROWS - 1; i++)
		{
			res[i] = math::subvector<T, COLS, EXCLUDE_COL>(mat[i + 1]);
		}

		return res;
	}

	template <typename T, size_t SIDE>
	T inline minor(const base_matrix<T, SIDE, SIDE> & mat, size_t column, size_t row)
	{
		return math::determinant(submatrix(mat, column, row));
	}

	template <typename T, size_t SIDE, size_t EXCLUDE_COL, size_t EXCLUDE_ROW>
	T inline minor(const base_matrix<T, SIDE, SIDE> & mat)
	{
		static_assert(EXCLUDE_COL >= 0, "EXCLUDE_COL can't be negative");
		static_assert(EXCLUDE_ROW >= 0, "EXCLUDE_ROW can't be negative");
		static_assert(EXCLUDE_COL < SIDE, "EXCLUDE_COL must be less than COLS");
		static_assert(EXCLUDE_ROW < SIDE, "EXCLUDE_ROW must be less than ROWS");

		return math::determinant(submatrix<T, SIDE, SIDE, EXCLUDE_COL, EXCLUDE_ROW>(mat));
	}

	template <typename T>
	T inline determinant(const base_matrix<T, 1, 1> & mat)
	{
		return mat[0][0];
	}

	template <typename T>
	T determinant(const base_matrix<T, 2, 2> & mat)
	{
		return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
	}

	template <typename T>
	T determinant(const base_matrix<T, 3, 3> & mat)
	{
		return mat[0][0] * mat[1][1] * mat[2][2] + mat[0][1] * mat[1][2] * mat[2][0] + mat[0][2] * mat[1][0] * mat[2][1]
		- mat[0][2] * mat[1][1] * mat[2][0] - mat[0][1] * mat[1][0] * mat[2][2] - mat[0][0] * mat[1][2] * mat[2][1];
	}

	template <typename T, size_t SIDE>
	T inline determinant(const base_matrix<T, SIDE, SIDE> & mat)
	{
		return determinant_iter<T, SIDE, 0, SIDE - 1>::determinant(mat);
	}

	template <typename T, size_t SIDE, size_t COLUMN, size_t COLUMNS_LEFT>
	class determinant_iter
	{
	public:
		enum
		{
			columns_left = COLUMNS_LEFT,
			column = COLUMN,
			side = SIDE
		};

		static T determinant(const base_matrix<T, SIDE, SIDE> & mat)
		{
			return mat[0][COLUMN] * minor<T, SIDE, COLUMN, 0>(mat) - determinant_iter<T, SIDE, COLUMN + 1, COLUMNS_LEFT - 1>::determinant(mat);
		}
	};

	template <typename T, size_t SIDE, size_t COLUMN>
	class determinant_iter<T, SIDE, COLUMN, 0>
	{
	public:
		enum
		{
			column = COLUMN,
			side = SIDE
		};

		static T determinant(const base_matrix<T, SIDE, SIDE> & mat)
		{
			return mat[0][COLUMN] * minor<T, SIDE, COLUMN, 0>(mat);
		}
	};


	template <typename T, size_t SIDE>
	base_matrix<T, SIDE, SIDE> inline inverse(const base_matrix<T, SIDE, SIDE> & mat)
	{
		base_matrix<T, SIDE, SIDE> res;

		T row_sign = T(1);
		for (size_t j = 0; j < SIDE; j++, row_sign = -row_sign)
		{
			T cell_sign = row_sign;
			for (size_t i = 0; i < SIDE; i++, cell_sign = -cell_sign)
			{
				res[j][i] = cell_sign * minor(mat, j, i);
			}
		}

		return res / determinant(mat);
	}

	template <typename T, size_t COLS, size_t ROWS>
	base_matrix<T, ROWS, COLS> inline transpose(const base_matrix<T, COLS, ROWS> & mat)
	{
		base_matrix<T, ROWS, COLS> res;
		for (size_t j = 0; j < ROWS; j++)
		{
			for (size_t i = 0; i < COLS; i++)
			{
				res[i][j] = mat[j][i];
			}
		}

		return res;
	}

	template <typename T, size_t SIDE>
	base_matrix<T, SIDE, SIDE> inline inverse_transpose(const base_matrix<T, SIDE, SIDE> & mat)
	{
		base_matrix<T, SIDE, SIDE> res;

		T row_sign = T(1);
		for (size_t j = 0; j < SIDE; j++, row_sign = -row_sign)
		{
			T cell_sign = row_sign;
			for (size_t i = 0; i < SIDE; i++, cell_sign = -cell_sign)
			{
				res[j][i] = cell_sign * minor(mat, i, j);
			}
		}

		return res / determinant(mat);
	}

	// last column is treated as translation and ignored
	template <typename T, size_t COLS, size_t ROWS>
	inline vector<T, ROWS> extractBoundsScale(const base_matrix<T, COLS, ROWS> & matrix)
	{
		vector<T, ROWS> scale;
		for (int i = 0; i < ROWS; i++)
		{
			scale[i] = length(matrix[i].reduce());
		}
		return scale;
	}

	template <typename T, size_t COLS, size_t ROWS>
	inline vector<T, ROWS - 1> extractTranslate(const base_matrix<T, COLS, ROWS> & matrix)
	{
		auto amount = matrix.get_column(ROWS - 1).reduce();
		return amount;
	}
}