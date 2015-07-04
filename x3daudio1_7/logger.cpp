#include "stdafx.h"
#include "logger.h"
#include <fstream>
#include <memory>

std::unique_ptr<std::ostream> stream;

void logger::log(std::string message)
{
	if (!stream)
	{
		stream = std::unique_ptr<std::ostream>(new std::ofstream("log.txt"));
	}

	*stream << message << std::endl << std::flush;
	stream->flush();
}
