#include "stdafx.h"
#include "logger.h"
#include <fstream>
#include <memory>

std::unique_ptr<std::wostream> stream;

void logger::details::log(const std::wstring & message)
{
	if (!stream)
	{
		stream = std::unique_ptr<std::wostream>(new std::wofstream("log.txt"));
	}

	*stream << message << std::endl << std::flush;
	stream->flush();
}
