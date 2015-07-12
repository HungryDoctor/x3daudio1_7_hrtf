#pragma once
#include <string>
#include <sstream>

namespace logger
{
	void log(std::wstring message);

	template <typename T1, typename ... Ts>
	void log(T1 t1, Ts ... ts)
	{
		std::wstringstream ss;
		details::apped_to_stream(ss, t1, ts ...);
		log(ss.str());
	}


	namespace details
	{
		template <typename T1, typename ... Ts>
		void apped_to_stream(std::wstringstream & ss, T1 t1, Ts ... ts)
		{
			ss << t1;
			apped_to_stream(ss, ts ...);
		}

		template <typename T1>
		void apped_to_stream(std::wstringstream & ss, T1 t1)
		{
			ss << t1;
		}
	}
}
