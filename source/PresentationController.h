#pragma once

#include "PresentPass.h"

class PresentationController
{
public:
	PresentationController(Image& sampler);
	~PresentationController();

	void prepareExecution();
	// 
	void present();

	std::vector<Image*>& getImages();
	std::vector<Pass*>& getPasses();
	VkDescriptorSet& getDescriptorSet();

private:
	VkSwapchainKHR swapChain;
	std::vector<Image*> images;
	std::vector<Pass*> passes;

	//std::vector<VkCommandBuffer> commandBuffersPresent;
	//std::vector<VkImage> swapChainImages;
	//std::vector<VkImageView> swapChainImageViews;
	//std::vector<VkFramebuffer> swapChainFramebuffers;
	//VkFormat swapChainImageFormat;
	//VkExtent2D swapChainExtent;

	// TODO what is a "frame?" Right now only swapchain images are duplicated. What happens if we have a copy of every resource per frame?
	void createSyncObjects();
	std::vector<VkFence> inFlightFences;
	size_t currentFrame = 0;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
};