#pragma once

#include <chrono>

class TimeSupport
{
public:
	static void update();

	// returns the elapsed time in seconds since the last frame
	static float getFrameTime();

	// returns seconds since epoch from system clock
	//static float getCurrentTime();
};

