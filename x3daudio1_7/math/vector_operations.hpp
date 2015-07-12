#pragma once

// fucking windows.h
#undef max
#undef min

#include "vector.hpp"
#include <algorithm>


namespace math
{
	template <typename T, size_t N>
	bool inline operator ==( const vector<T, N> & one, const vector<T, N> & another)
	{
		bool equals = true;
		iterate<0, N - 1>([&](size_t i)
		{
			equals &= one[i] == another[i];
		});
		return equals;
	}

	template <typename T, size_t N>
	bool inline operator !=( const vector<T, N> & one, const vector<T, N> & another)
	{
		return !(one == another);
	}

	template <typename T, size_t N>
	vector<T, N> inline operator +( const vector<T, N> & one, const vector<T, N> & another)
	{
		vector<T, N> res;
		iterate<0, N - 1>([&](size_t i)
		{
			res[i] = one[i] + another[i];
		});
		return res;
	}

	template <typename T, size_t N>
	vector<T, N> inline operator +=( vector<T, N> & one, const vector<T, N> & another)
	{
		iterate<0, N - 1>([&](size_t i)
		{
			one[i] += another[i];
		});
		return one;
	}

	template <typename T, size_t N>
	vector<T, N> inline operator -( const vector<T, N> & one, const vector<T, N> & another)
	{
		vector<T, N> res;
		iterate<0, N - 1>([&](size_t i)
		{
			res[i] = one[i] - another[i];
		});
		return res;
	}

	template <typename T, size_t N>
	vector<T, N> inline operator -=( vector<T, N> & one, const vector<T, N> & another)
	{
		iterate<0, N - 1>([&](size_t i)
		{
			one[i] -= another[i];
		});
		return one;
	}

	template <typename T, size_t N>
	vector<T, N> inline operator -( const vector<T, N> & one)
	{
		vector<T, N> res;
		iterate<0, N - 1>([&](size_t i)
		{
			res[i] = -one[i];
		});
		return res;
	}

	template <typename T, size_t N>
	vector<T, N> inline operator *( const vector<T, N> & one, const vector<T, N> & another)
	{
		vector<T, N> res;
		iterate<0, N - 1>([&](size_t i)
		{
			res[i] = one[i] * another[i];
		});
		return res;
	}

	template <typename T, size_t N>
	vector<T, N> inline operator *( const vector<T, N> & one, const T & scalar)
	{
		vector<T, N> res;
		iterate<0, N - 1>([&](size_t i)
		{
			res[i] = one[i] * scalar;
		});
		return res;
	}

	template <typename T, size_t N>
	vector<T, N> inline operator *( const T & scalar, const vector<T, N> & another)
	{
		return another * scalar;
	}

	template <typename T, size_t N>
	vector<T, N> inline operator /( const vector<T, N> & one, const vector<T, N> & another)
	{
		vector<T, N> res;
		iterate<0, N - 1>([&](size_t i)
		{
			res[i] = one[i] / another[i];
		});
		return res;
	}

	template <typename T, size_t N>
	vector<T, N> inline operator /( const vector<T, N> & one, const T & scalar)
	{
		T recipr = T(1) / scalar;
		vector<T, N> res;
		iterate<0, N - 1>([&](size_t i)
		{
			res[i] = one[i] * recipr;
		});
		return res;
	}

	template <typename T, size_t N>
	vector<T, N> inline operator /( const T & scalar, const vector<T, N> & another )
	{
		vector<T, N> res;
		iterate<0, N - 1>([&](size_t i)
		{
			res[i] = scalar / another[i];
		});
		return res;
	}

}