#pragma once

#include <random>

class RandomEngine
{
public:
	// TODO : implement safe multithreading!
	static std::default_random_engine & engine() { return _engine; }

private:
	RandomEngine(void) {}
	~RandomEngine(void) {}

	static std::default_random_engine _engine;

};

