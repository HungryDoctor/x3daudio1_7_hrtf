#pragma once

#include "vector.hpp"
#include "vector_operations.hpp"
#include <crtdefs.h>

namespace math
{
	template <typename T>
	class base_matrix<T, 4, 4>
	{
	public:
		// Constructs identity matrix
		base_matrix()
		{
			m_rows[0][0] = T(1);
			m_rows[0][1] = T(0);
			m_rows[0][2] = T(0);
			m_rows[0][3] = T(0);

			m_rows[1][0] = T(0);
			m_rows[1][1] = T(1);
			m_rows[1][2] = T(0);
			m_rows[1][3] = T(0);

			m_rows[2][0] = T(0);
			m_rows[2][1] = T(0);
			m_rows[2][2] = T(1);
			m_rows[2][3] = T(0);

			m_rows[3][0] = T(0);
			m_rows[3][1] = T(0);
			m_rows[3][2] = T(0);
			m_rows[3][3] = T(1);
		}

		base_matrix(const T & x0, const T & y0, const T & z0, const T & w0,
		            const T & x1, const T & y1, const T & z1, const T & w1,
					const T & x2, const T & y2, const T & z2, const T & w2,
					const T & x3, const T & y3, const T & z3, const T & w3
			)
		{
			m_rows[0][0] = x0;
			m_rows[0][1] = y0;
			m_rows[0][2] = z0;
			m_rows[0][3] = w0;

			m_rows[1][0] = x1;
			m_rows[1][1] = y1;
			m_rows[1][2] = z1;
			m_rows[1][3] = w1;

			m_rows[2][0] = x2;
			m_rows[2][1] = y2;
			m_rows[2][2] = z2;
			m_rows[2][3] = w2;

			m_rows[3][0] = x3;
			m_rows[3][1] = y3;
			m_rows[3][2] = z3;
			m_rows[3][3] = w3;
		}

		template<typename TOther>
		base_matrix(const base_matrix<TOther, 4, 4> & other)
		{
			for (size_t j = 0; j < 4; j++)
			{
				for (size_t i = 0; i < 4; i++)
				{
					m_rows[j][i] = other[j][i];
				}
			}
		}

		size_t num_rows() const { return 4; }
		size_t num_cols() const { return 4; }

		vector<T, 4> & operator[](std::size_t index)
		{
			return m_rows[index];
		}

		const vector<T, 4> & operator[](std::size_t index) const
		{
			return m_rows[index];
		}

		vector<T, 4> get_column(std::size_t index) const
		{
			vector<T, 4> res;
			res[0] = m_rows[0][index];
			res[1] = m_rows[1][index];
			res[2] = m_rows[2][index];
			res[3] = m_rows[3][index];
			return res;
		}

	private:
		vector<T, 4> m_rows[4];
	};
}