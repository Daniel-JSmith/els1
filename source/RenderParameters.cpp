#include "RenderParameters.h"

#include <iostream>

RenderParameters RenderParameters::instance;

const VkExtent2D RenderParameters::RENDER_RESOLUTION = { 1920 / 6, 1080 / 6 };
const VkExtent2D RenderParameters::PRESENT_RESOLUTION = { 1920 / 2, 1080 / 2};

const int RenderParameters::RENDER_CENTER_X = RENDER_RESOLUTION.width / 2;
const int RenderParameters::RENDER_CENTER_Y = RENDER_RESOLUTION.height / 2;

const int RenderParameters::MAX_FRAMES_IN_FLIGHT = 2;

RenderParameters::RenderParameters() : numSamplesPerPixel(1)
{
}

RenderParameters& RenderParameters::getInstance()
{
	return instance;
}

void RenderParameters::registerParameterChangeCallback(void(*callback)())
{
	this->callback = callback;
}

int RenderParameters::getNumSamplesPerPixel() const
{
	return numSamplesPerPixel;
}

void RenderParameters::changeNumSamplesPerPixel(int delta)
{
	
	if (numSamplesPerPixel + delta >= 0)
	{
		numSamplesPerPixel += delta;
	}
	std::cout << "render parameters: " << numSamplesPerPixel << std::endl;
	if (callback)
	{
		callback();
	}
	
}