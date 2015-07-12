#pragma once

//#include "base_vector3.hpp"
#include "base_matrix4x4.hpp"
#include "base_matrix_operations.hpp"

#include <algorithm>

#define M_PI 3.1415926535897932384626433832795

namespace math
{
	template <typename T>
	base_matrix<T, 4, 4> inline translate(const vector<T, 3> & vec)
	{
		base_matrix<T, 4, 4> res;

		res[0][3] = vec[0];
		res[1][3] = vec[1];
		res[2][3] = vec[2];

		return res;
	}

	template <typename T>
	base_matrix<T, 4, 4> inline scale(const vector<T, 3> & vec)
	{
		base_matrix<T, 4, 4> res;

		res[0][0] = vec[0];
		res[1][1] = vec[1];
		res[2][2] = vec[2];

		return res;
	}

	template <typename T>
	base_matrix<T, 4, 4> inline rotate(const vector<T, 3> & axis, T radians)
	{
		T sinTheta = std::sin(radians);
		T cosTheta = std::cos(radians);

		base_matrix<T, 4, 4> a_star(
			T(0),		-axis[2],	axis[1],	T(0),
			axis[2],	T(0),		-axis[0],	T(0),
			-axis[1],	axis[0],	T(0),		T(0),
			T(0),		T(0),		T(0),		T(0)
			);

		base_matrix<T, 4, 4> a_aT(
			axis[0] * axis[0], axis[0] * axis[1], axis[0] * axis[2], T(0),
			axis[1] * axis[0], axis[1] * axis[1], axis[1] * axis[2], T(0),
			axis[2] * axis[0], axis[2] * axis[1], axis[2] * axis[2], T(0),
			T(0),                T(0),                T(0),                T(0)
			);

		auto res = base_matrix<T, 4, 4>() * cosTheta + a_aT * (1 - cosTheta) + a_star * sinTheta;
		res[3][3] = T(1);
		return res;
	}

	template <typename T>
	base_matrix<T, 4, 4> inline rotate_degrees(const vector<T, 3> & axis, T degrees)
	{
		T radians = T(M_PI) / T(180) * degrees;
		return rotate(axis, radians);
	}
}