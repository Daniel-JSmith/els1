#pragma once

#ifndef GLFW_INCLUDE_VULKAN
	#define GLFW_INCLUDE_VULKAN
#endif // !GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

// stores number of samples per pixel and other variables related to rendering
class RenderParameters
{
public:
	// the result is internally rendered at RENDER_RESOLUTION displayed at PRESENT_RESOLUTION
	static const VkExtent2D RENDER_RESOLUTION;
	static const VkExtent2D PRESENT_RESOLUTION;

	static const int RENDER_CENTER_X;
	static const int RENDER_CENTER_Y;

	static const VkFormat renderTargetFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
	static const VkFormat textureFormat = VK_FORMAT_R8G8B8A8_SRGB;

	static const int MAX_FRAMES_IN_FLIGHT;

	void registerParameterChangeCallback( void (*callback)());

	static RenderParameters& getInstance();
	int getNumSamplesPerPixel() const;
	void changeNumSamplesPerPixel(int delta);

private:
	static RenderParameters instance;
	int numSamplesPerPixel;
	void(*callback)();

	RenderParameters();

	

	
};