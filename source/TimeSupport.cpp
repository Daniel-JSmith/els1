#include "TimeSupport.h"

namespace
{
	std::chrono::high_resolution_clock::time_point previousTime = std::chrono::high_resolution_clock::now();
	float deltaTime = 0;
}

void TimeSupport::update()
{
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

	deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - previousTime).count();

	previousTime = currentTime;
}

float TimeSupport::getFrameTime()
{
	return deltaTime;
}