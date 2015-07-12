#pragma once

//#define MATH_ENABLE_SSE

#include "vector.hpp"
#include "vector_operations.hpp"
#include "vector_functions.hpp"
//#include "base_vector3.hpp"
#include "vector3_operations.hpp"
//#include "vector4.hpp"
#if defined(MATH_ENABLE_SSE)
#include "sse_float_vector3.hpp"
#include "sse_float_vector3_operations.hpp"
#include "sse_float_vector4.hpp"
#include "sse_float_vector4_operations.hpp"
#endif
#include "base_matrix_operations.hpp"
#include "base_matrix_functions.hpp"
#include "base_matrix4x4_transfrom.hpp"
#include "random.hpp"


namespace math
{
	typedef vector<float, 3> vector3f;
	typedef vector<float, 4> vector4f;
	typedef vector<double, 3> vector3d;
	typedef vector<double, 4> vector4d;

	typedef base_matrix<float, 3, 3> matrix3f;
	typedef base_matrix<float, 4, 4> matrix4f;
	typedef base_matrix<double, 3, 3> matrix3d;
	typedef base_matrix<double, 4, 4> matrix4d;


	//////////////////////////////////////////////////////////////////////////
	// scalar stuff
	//////////////////////////////////////////////////////////////////////////
	template <typename T>
	T sign(T val)
	{
		return val >= 0 ? T(1) : T(-1);
	}
}