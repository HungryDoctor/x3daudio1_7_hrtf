#pragma once

#include "vector.hpp"

#include <algorithm>

namespace math
{

	template <typename T>
	vector<T, 3> inline cross( const vector<T, 3> & one, const vector<T, 3> & other)
	{
		return vector<T, 3>
			(
			one[1] * other[2] - one[2] * other[1],
			one[2] * other[0] - one[0] * other[2],
			one[0] * other[1] - one[1] * other[0]
			);
	}

}