#include "presentationController.h"
#include "GPUStructs.h"

PresentationController::PresentationController(Image& sampler)
{
	swapChain = VulkanCoreSupport::getInstance().swapChain;

	uint32_t numSwapChainImages = 0;
	std::vector<VkImage> vulkanImages;

	// create images
	vkGetSwapchainImagesKHR(VulkanCoreSupport::getInstance().device, swapChain, &numSwapChainImages, nullptr);
	vulkanImages.resize(numSwapChainImages);
	images.resize(numSwapChainImages);
	passes.resize(numSwapChainImages);
	vkGetSwapchainImagesKHR(VulkanCoreSupport::getInstance().device, swapChain, &numSwapChainImages, vulkanImages.data());

	// create Image object wrappers
	for (size_t i = 0; i < numSwapChainImages; i++)
	{
		images.at(i) = new Image(vulkanImages.at(i), VulkanCoreSupport::getInstance().swapChainImageFormat, VulkanCoreSupport::getInstance().swapChainExtent);
	}

	//create a Pass object for each image
	for (int i = numSwapChainImages - 1; i >= 0; i--)
	{
		passes.at(i) = new PresentPass(sampler, *images.at(i));
	}
			
	createSyncObjects();
}

PresentationController::~PresentationController()
{
	VkDevice device = VulkanCoreSupport::getInstance().getDevice();

	for (int i = 0; i < images.size(); i++)
	{
		delete images.at(i);
		delete passes.at(i);
	}

	for (size_t i = 0; i < VulkanCoreSupport::getInstance().numSwapChainImages; i++)
	{
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
}

void PresentationController::prepareExecution()
{
	for (Pass* pass : passes)
	{
		pass->prepareExecution({}, {});
	}
}

void PresentationController::present()
{
	VkDevice device = VulkanCoreSupport::getInstance().getDevice();

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, VulkanCoreSupport::getInstance().swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		//recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image");
	}

	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	// copy complete frame into swapchain image

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &passes[currentFrame]->getCommandBuffer();
	
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	   

	if (vkQueueSubmit(VulkanCoreSupport::getInstance().graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer");
	}

	// submit to present queue

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { VulkanCoreSupport::getInstance().swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(VulkanCoreSupport::getInstance().presentQueue, &presentInfo);

	//if (result == VK_ERROR_OUT_OF_DATE_KHR
	//	|| result == VK_SUBOPTIMAL_KHR
	//	|| framebufferResized)
	//{
	//	framebufferResized = false;
	//	/*recreateSwapChain();*/
	//}
	//else if (result != VK_SUCCESS)
	//{
	//	throw std::runtime_error("failed to present swap chain images");
	//}

	currentFrame = (currentFrame + 1) % VulkanCoreSupport::getInstance().numSwapChainImages;
}

void PresentationController::createSyncObjects()
{
	uint32_t numSwapChainImages = VulkanCoreSupport::getInstance().numSwapChainImages;
	VkDevice device = VulkanCoreSupport::getInstance().getDevice();

	imageAvailableSemaphores.resize(numSwapChainImages);
	renderFinishedSemaphores.resize(numSwapChainImages);
	inFlightFences.resize(numSwapChainImages);
	
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (size_t i = 0; i < numSwapChainImages; i++)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
			|| vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
			|| vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects");
		}
	}
}


std::vector<Image*>& PresentationController::getImages()
{
	return images;
}

std::vector<Pass*>& PresentationController::getPasses()
{
	return passes;
}

VkDescriptorSet& PresentationController::getDescriptorSet()
{
	return passes[0]->getDescriptorSet();
}
