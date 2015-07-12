#pragma once

#include "helper.hpp"

#include <cstddef>

namespace math
{
	// Represents vector in space of TSpace^DIMENSIONS.
	template<typename TSpace, size_t DIMENSIONS>
	class vector
	{
	public:

		enum { dimensions = DIMENSIONS };

		vector()
		{
			iterate<0, dimensions - 1>([&] (size_t i) { m_elements[i] = TSpace(0); });
		}

		template <typename TOther>
		vector(const vector<TOther, DIMENSIONS> & other )
		{
			iterate<0, dimensions - 1>([&] (size_t i) { m_elements[i] = TSpace(other[i]); });
		}

		template <typename TOther>
		explicit vector(const TOther other[])
		{
			iterate<0, dimensions - 1>([&] (size_t i) { m_elements[i] = TSpace(other[i]); });
		}

		template <typename TValue>
		explicit vector(const TValue & value)
		{
			iterate<0, dimensions - 1>([&] (size_t i) { m_elements[i] = TSpace(value); });
		}

		template <typename TFirstValue, typename ... TValues>
		explicit vector(const TFirstValue & first_value, const TValues & ... values)
		{
			static_assert(sizeof ... (values) == DIMENSIONS - 1, "Number of arguments does not match dimensionality of the vector!");

			setValues(m_elements, first_value, values ...);
		}

		template <typename ... TValues>
		explicit vector(const vector<TSpace, DIMENSIONS - sizeof ... (TValues)> & vec, const TValues & ... values)
		{
			static_assert(sizeof ... (values) < DIMENSIONS, "Too many arguments!");

			iterate<0, DIMENSIONS - sizeof ... (values) - 1>([&] (size_t i) { m_elements[i] = vec[i]; });

			setValues(m_elements + DIMENSIONS - sizeof ... (values), values ...);
		}

		const TSpace & operator[](std::size_t index) const
		{
			return m_elements[index];
		}

		TSpace & operator[](std::size_t index)
		{
			return m_elements[index];
		}

		vector<TSpace, DIMENSIONS - 1> reduce() const
		{
			return vector<TSpace, DIMENSIONS - 1>(m_elements);
		}

		template <typename TNewSpace>
		vector<TNewSpace, DIMENSIONS> cast()
		{
			return vector<TNewSpace, DIMENSIONS>(*this);
		}

	private:
		TSpace m_elements[DIMENSIONS];
	};


}