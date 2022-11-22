#include "Image.h"

#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const std::map<AccessSpecifier::OPERATION, VkImageLayout> Image::REQUIRED_LAYOUTS = 
{
	{AccessSpecifier::OPERATION::COLOR_ATTACHMENT_OUTPUT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
	{AccessSpecifier::OPERATION::SHADER_READ, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
	{AccessSpecifier::OPERATION::TRANSFER_DESTINATION, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL},
	{AccessSpecifier::OPERATION::TRANSFER_SOURCE, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL},
	{AccessSpecifier::OPERATION::PREPARE_FOR_PRESENTATION, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL},
	{AccessSpecifier::OPERATION::PRESENT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR}
};


void createImage(uint32_t width, uint32_t height, VkFormat format,
	VkImageTiling tiling, VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties, VkImage& image,
	VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	if (vkCreateImage(VulkanCoreSupport::getInstance().getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image");
	}

	// allocate memory
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(VulkanCoreSupport::getInstance().getDevice(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = VulkanCoreSupport::getInstance().findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(VulkanCoreSupport::getInstance().getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory");
	}

	vkBindImageMemory(VulkanCoreSupport::getInstance().getDevice(), image, imageMemory, 0);
}

void createImageView(VkImage image, VkFormat format, VkImageView& imageView)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkDevice device = VulkanCoreSupport::getInstance().device;

	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image view");
	}
}

void createSampler(VkSampler& sampler)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(VulkanCoreSupport::getInstance().getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create sampler");
	}
}

// support for constructors
void Image::init(const VkFormat& format, const VkImageUsageFlags& usage, const VkMemoryPropertyFlags& memoryProperties, const VkExtent2D& extent)
{
	this->format = format;
	this->usage = usage;
	this->memoryProperties = memoryProperties;
	this->extent = extent;

	// create image
	createImage(extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, usage, memoryProperties, image, imageMemory);
	responsibleForImageDestruction = true;

	// create view
	createImageView(image, format, imageView);

	// create sampler
	createSampler(sampler);
}

// default sampler
Image::Image(const VkFormat& format, const VkImageUsageFlags& usage, const VkMemoryPropertyFlags& memoryProperties, const VkExtent2D& extent) : Resource(USE::SAMPLER, Resource::ACCESS_PROPERTY::GPU_PREFERRED), currentLayout(VK_IMAGE_LAYOUT_UNDEFINED)
{
	init(format, usage, memoryProperties, extent);
}

Image::Image(const std::string& path, VkFormat textureFormat) : Resource(USE::SAMPLER, Resource::ACCESS_PROPERTY::GPU_PREFERRED), currentLayout(VK_IMAGE_LAYOUT_UNDEFINED)
{
	// Read data from texture file
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	// TODO what is this?
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	if (!pixels)
	{
		throw std::runtime_error("failed to load texture");
	}

	// Create temp stagingBuffer
	Buffer stagingBuffer(imageSize, USE::TRANSFER_SOURCE, ACCESS_PROPERTY::CPU_PREFERRED, pixels);

	stbi_image_free(pixels);

	init(textureFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkExtent2D{ static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) });
	transitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer);
	transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

Image::Image(VkImage& image, VkFormat format, VkExtent2D extent) : Resource(USE::SAMPLER, Resource::ACCESS_PROPERTY::GPU_PREFERRED), currentLayout(VK_IMAGE_LAYOUT_UNDEFINED)
{
	this->image = image;
	this->format = format;
	this->extent = extent;
	createImageView(image, format, imageView);
	createSampler(sampler);
}


Image::~Image()
{
	vkDestroySampler(VulkanCoreSupport::getInstance().getDevice(), sampler, nullptr);
	vkDestroyImageView(VulkanCoreSupport::getInstance().getDevice(), imageView, nullptr);

	if (responsibleForImageDestruction)
	{
		vkDestroyImage(VulkanCoreSupport::getInstance().getDevice(), image, nullptr);
		vkFreeMemory(VulkanCoreSupport::getInstance().getDevice(), imageMemory, nullptr);
	}
}

void Image::transitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = VulkanCoreSupport::getInstance().beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = 0;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition");
	}

	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage,
		destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	VulkanCoreSupport::getInstance().endSingleTimeCommands(commandBuffer);
}

void Image::copyBufferToImage(VkBuffer buffer)
{
	VkCommandBuffer commandBuffer = VulkanCoreSupport::getInstance().beginSingleTimeCommands();
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		extent.width,
		extent.height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	VulkanCoreSupport::getInstance().endSingleTimeCommands(commandBuffer);
}

void Image::copyBufferToImage(const Buffer& buffer)
{
	VkCommandBuffer commandBuffer = VulkanCoreSupport::getInstance().beginSingleTimeCommands();
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		extent.width,
		extent.height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer.getBufferObject(),
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	VulkanCoreSupport::getInstance().endSingleTimeCommands(commandBuffer);
}

void Image::createLayoutBinding(int index, VkShaderStageFlags stage, VkDescriptorSetLayoutBinding& out) const
{
	out.binding = index;
	out.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	out.descriptorCount = 1;
	out.stageFlags = stage;
	out.pImmutableSamplers = nullptr;
}

void Image::createDescriptorWrite(int index, const VkDescriptorSet& descriptorSet, VkWriteDescriptorSet& out) const
{
	VkDescriptorImageInfo samplerInfo{};
	samplerInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	samplerInfo.imageView = imageView;
	samplerInfo.sampler = sampler;

	out.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	out.dstSet = descriptorSet;
	out.dstBinding = index;
	out.dstArrayElement = 0;
	out.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	out.descriptorCount = 1;
	out.pBufferInfo = nullptr;
	out.pImageInfo = &samplerInfo;
	out.pTexelBufferView = nullptr;
	out.pNext = nullptr;

	vkUpdateDescriptorSets(VulkanCoreSupport::getInstance().getDevice(), 1, &out, 0, nullptr);
}

// TODO code duplication

void Image::prepareForAccess(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier)
{
	VkImageLayout requiredLayout = REQUIRED_LAYOUTS.at(accessSpecifier.operation);
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;
	barrier.oldLayout = currentLayout;
	barrier.newLayout = requiredLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		0,
		0, VK_NULL_HANDLE,
		0, VK_NULL_HANDLE,
		1, &barrier
	);

	currentLayout = requiredLayout;
}

void Image::insertWARBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier)
{
	VkImageLayout requiredLayout = REQUIRED_LAYOUTS.at(accessSpecifier.operation);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;
	barrier.oldLayout = currentLayout;
	barrier.newLayout = requiredLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		0,
		0, VK_NULL_HANDLE,
		0, VK_NULL_HANDLE,
		1, &barrier
	);

	currentLayout = requiredLayout;
}

void Image::insertRAWBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier)
{
	VkImageLayout requiredLayout = REQUIRED_LAYOUTS.at(accessSpecifier.operation);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	// TODO we are assuming these access types. We should store access types in AccessSpecifier to support other dependencies e.g. input attachment read after shader write
	barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout = currentLayout;
	barrier.newLayout = requiredLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		0,
		0, VK_NULL_HANDLE,
		0, VK_NULL_HANDLE,
		1, &barrier
	);

	currentLayout = requiredLayout;
}

void Image::insertFinalBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier)
{

}

void Image::getAttachmentDescription(AccessSpecifier currentAccess, AccessSpecifier futureAccess, VkAttachmentDescription& attachmentDescription, VkAttachmentReference& attachmentReference)
{
	VkImageLayout requiredLayout = REQUIRED_LAYOUTS.at(currentAccess.operation);

	attachmentDescription = VkAttachmentDescription{};
	attachmentDescription.format = format;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	attachmentDescription.finalLayout = REQUIRED_LAYOUTS.at(futureAccess.operation);


	attachmentReference = VkAttachmentReference{};
	attachmentReference.attachment = 0;
	attachmentReference.layout = requiredLayout;

	currentLayout = attachmentDescription.finalLayout;
}

VkImage& Image::getImage()
{
	return image;
}
VkImageView& Image::getImageView()
{
	return imageView;
}
VkSampler& Image::getSampler()
{
	return sampler;
}

const VkFormat& Image::getFormat()
{
	return format;
}

const VkExtent2D& Image::getExtent()
{
	return extent;
}