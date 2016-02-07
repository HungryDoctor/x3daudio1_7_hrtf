#pragma once
#include <string>
#include <sstream>

namespace logger
{
	namespace details
	{
		void log(const std::wstring & message);

		template <typename T1, typename ... Ts>
		void apped_to_stream(std::wstringstream & ss, const T1 & t1, const Ts & ... ts)
		{
			ss << t1;
			apped_to_stream(ss, ts ...);
		}

		template <typename T1>
		void apped_to_stream(std::wstringstream & ss, const T1 & t1)
		{
			ss << t1;
		}

		template <typename T1, typename ... Ts>
		void logDebug(const T1 & t1, const Ts & ... ts)
		{
#ifdef _DEBUG
			std::wstringstream ss;
			details::apped_to_stream(ss, t1, ts ...);
			details::log(ss.str());
#endif
		}

		template <typename T1, typename ... Ts>
		void logRelease(const T1 & t1, const Ts & ... ts)
		{
			std::wstringstream ss;
			details::apped_to_stream(ss, t1, ts ...);
			details::log(ss.str());
		}
	}
}
