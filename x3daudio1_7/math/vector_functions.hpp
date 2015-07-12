#pragma once

#include "vector.hpp"
#include <algorithm>


namespace math
{
	template <typename T, size_t N>
	T inline dot( const vector<T, N> & one, const vector<T, N> & another)
	{
		static_assert(N > 0, "Vector must have at least one component.");

		T res = one[0] * another[0];
		iterate<1, N - 1>([&](size_t i)
		{
			res += one[i] * another[i];
		});

		return res;
	}

	// Computes dot product and populates vector with this value.
	template <typename T, size_t N>
	vector<T, N> inline dot_vec( const vector<T, N> & one, const vector<T, N> & another)
	{
		static_assert(N > 0, "Vector must have at least one component.");

		return vector<T, N>(dot(one, another));
	}

	template <typename T, size_t N>
	T inline length2( const vector<T, N> & one )
	{
		return dot(one, one);
	}

	template <typename T, size_t N>
	T inline length( const vector<T, N> & one )
	{
		return std::sqrt(length2(one));
	}

	// Not guaranteed to be precise but may be faster.
	template <typename T, size_t N>
	T inline fast_length( const vector<T, N> & one )
	{
		return length(one);
	}

	template <typename T, size_t N>
	T inline inv_length( const vector<T, N> & one )
	{
		return T(1) / length(one);
	}

	// Not guaranteed to be precise but may be faster.
	template <typename T, size_t N>
	T inline fast_inv_length( const vector<T, N> & one )
	{
		return T(1) / fast_length(one);
	}

	template <typename T, size_t N>
	vector<T, N> inline normalize( const vector<T, N> & one )
	{
		return one * inv_length(one);
	}

	template <typename T, size_t N>
	vector<T, N> inline fast_normalize( const vector<T, N> & one )
	{
		return one * fast_inv_length(one);
	}

	template <typename T, size_t N>
	vector<T, N> inline reciprocal( const vector<T, N> & one )
	{
		return T(1) / one;
	}

	// Not guaranteed to be precise but may be faster.
	template <typename T, size_t N>
	vector<T, N> inline fast_reciprocal( const vector<T, N> & one )
	{
		return reciprocal(one);
	}

	template <typename T, size_t N>
	vector<T, N> inline max( const vector<T, N> & one, const vector<T, N> & another)
	{
		vector<T, N> res;
		iterate<0, N - 1>([&](size_t i)
		{
			res[i] = std::max(one[i], another[i]);
		});
		return res;
	}

	template <typename T, size_t N>
	vector<T, N> inline min( const vector<T, N> & one, const vector<T, N> & another)
	{
		vector<T, N> res;
		iterate<0, N - 1>([&](size_t i)
		{
			res[i] = std::min(one[i], another[i]);
		});
		return res;
	}

	template <typename T, size_t N>
	vector<T, N> inline clamp( const vector<T, N> & vec, const vector<T, N> & low, const vector<T, N> & high)
	{
		return min(high, max(low, vec));
	}

	template <typename T, size_t N>
	vector<T, N> inline abs( const vector<T, N> & one)
	{
		vector<T, N> res;
		iterate<0, N - 1>([&](size_t i)
		{
			res[i] = std::abs(one[i]);
		});
		return res;
	}

	template <typename T, size_t N>
	vector<T, N - 1> inline subvector( const vector<T, N> & vec, size_t element)
	{
		vector<T, N - 1> res;
		for (size_t i = 0; i < element; i++)
		{
			res[i] = vec[i];
		}

		for (size_t i = element; i < N - 1; i++)
		{
			res[i] = vec[i + 1];
		}

		return res;
	}

	template <typename T, size_t N, size_t ELEMENT>
	vector<T, N - 1> inline subvector( const vector<T, N> & vec)
	{
		static_assert(N > 1, "N must be greater then 1!");
		static_assert(ELEMENT >= 0, "ELEMENT can't be negative!");
		static_assert(ELEMENT < N, "ELEMENT must be less than N!");

		vector<T, N - 1> res;
		iterate<0, int(ELEMENT) - 1>([&](size_t i)
		{
			res[i] = vec[i];
		});

		iterate<int(ELEMENT), int(N) - 2>([&](size_t i)
		{
			res[i] = vec[i + 1];
		});

		return res;
	}

	template <typename T, size_t N>
	vector<T, N> inline blend( const vector<T, N> & one, const vector<T, N> & another, const T & fraction)
	{
		T inv_fraction = T(1) - fraction;
		vector<T, N> res;
		iterate<0, N - 1>([&](size_t i)
		{
			res[i] = one[i] * inv_fraction + another[i] * fraction;
		});
		return res;
	}

	template <typename T, size_t N>
	vector<T, N> inline blend( const vector<T, N> & one, const vector<T, N> & another, const vector<T, N> & fractions)
	{
		auto inv_fraction = vector<T, N>(T(1)) - fractions;
		vector<T, N> res = one * inv_fraction + another * fractions;
		return res;
	}
}