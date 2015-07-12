#pragma once

#include <cmath>
#include <type_traits>

namespace math
{
	template <typename TArray, typename TFirst, typename ... TRest>
	void setValues(TArray array[], const TFirst & first, const TRest & ... rest)
	{
		array[0] = TArray(first);
		setValues(array + 1, rest ...);
	}

	template <typename TArray, typename TFirst>
	void setValues(TArray array[], const TFirst & first)
	{
		array[0] = TArray(first);
	}


	template <typename T>
	T saturate(const T & value, const T & min_value, const T & max_value)
	{
		return std::min(max_value, std::max(min_value, value));
	}

	template <typename TInt, typename TFloat>
	TInt fast_floor(const TFloat & float_value)
	{
		static_assert(std::is_floating_point<TFloat>::value, "Argument must be floating point!");
		static_assert(std::is_integral<TInt>::value && !std::is_same<TInt, bool>::value, "Return value must be of integral type but not bool.");
		return float_value >= TFloat(0) ? static_cast<TInt>(float_value) : static_cast<TInt>(float_value) - TInt(1);
	}

	// Implements static loop with templates
	template<int FROM, int TO, typename T>
	void iterate(T & func)
	{
		details::iterate<FROM, TO, T>(details::bool_tag<FROM <= TO>(), func);
	}

	namespace details
	{
		template <int level>
		struct tag { };

		template <bool VALUE>
		struct bool_tag { enum { value = VALUE }; };

		template<int FROM, int TO, typename T>
		void iterate(bool_tag<false>, T & func)
		{
			// NOOP
		}

		template<int FROM, int TO, typename T>
		void iterate(bool_tag<true>, T & func)
		{
			static_assert(FROM <= TO, "TO cannot be less than FROM!");
			details::index_iterator<FROM, TO>::iterate(func);
		}


		template<int FROM, int TO>
		struct index_iterator
		{
			enum { from = FROM, to = TO };

			template<typename T>
			static void iterate(T & func)
			{
				iterate(tag<from>(), func);
			}

		private:
			template<typename T, int INDEX>
			static void iterate(tag<INDEX>, T & func)
			{
				func(INDEX);
				iterate(tag<INDEX + 1>(), func);
			}

			template<typename T>
			static void iterate(tag<to>, T & func)
			{
				func(TO);
			}
		};

	}
}