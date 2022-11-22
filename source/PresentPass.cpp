#include "PresentPass.h"

PresentPass::PresentPass(Image& sourceImage, Image& swapChainImage) : Pass({}), sourceImage(sourceImage), swapChainImage(swapChainImage)
{

}

PresentPass::~PresentPass()
{

}

void PresentPass::prepareExecution(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors)
{
	Pass::prepareExecution(predecessors, successors);
	recordCommandBuffer(predecessors, successors);
}

void PresentPass::recordCommandBuffer(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors)
{
	startCommandBufferRecording(predecessors, successors);

	swapChainImage.prepareForAccess(commandBuffer, AccessSpecifier{ AccessSpecifier::ACCESS_TYPE::READ, AccessSpecifier::OPERATION::TRANSFER_DESTINATION });

	VkImageSubresourceLayers subresourceLayers{};
	subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceLayers.mipLevel = 0;
	subresourceLayers.baseArrayLayer = 0;
	subresourceLayers.layerCount = 1;

	VkImageBlit blitRegion{};
	blitRegion.srcSubresource = subresourceLayers;
	blitRegion.srcOffsets[0] = { 0, 0, 0 }; blitRegion.srcOffsets[1] = { static_cast<int32_t>(sourceImage.getExtent().width), static_cast<int32_t>(sourceImage.getExtent().height), 1 };
	blitRegion.dstSubresource = subresourceLayers;
	blitRegion.dstOffsets[0] = { 0, 0, 0 }; blitRegion.dstOffsets[1] = { static_cast<int32_t>(swapChainImage.getExtent().width), static_cast<int32_t>(swapChainImage.getExtent().height), 1 };

	vkCmdBlitImage(commandBuffer, sourceImage.getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapChainImage.getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_NEAREST);
	
	swapChainImage.prepareForAccess(commandBuffer, AccessSpecifier{ AccessSpecifier::ACCESS_TYPE::READ, AccessSpecifier::OPERATION::PRESENT});

	endCommandBufferRecording();
}