#pragma once

#include "new_math.hpp"

//#define DOUBLE_PRECISION

namespace math
{
#if defined (DOUBLE_PRECISION)
	typedef math::vector3d vector3;
	typedef math::vector4d vector4;
	typedef math::matrix3d matrix3;
	typedef math::matrix4d matrix4;
	typedef double space_real;
#else
	typedef math::vector3f vector3;
	typedef math::vector4f vector4;
	typedef math::matrix3f matrix3;
	typedef math::matrix4f matrix4;
	typedef float space_real;
#endif

	typedef math::vector3f color_rgb;
	typedef math::vector4f color_rgbx;
	typedef float color_real;
}