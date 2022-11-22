#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <map>

#include "RenderParameters.h"
#include "VulkanCoreSupport.h"
#include "Buffer.h"
#include "Resource.h"
#include "ResourceAccessSpecifier.h"

class Image : public Resource
{
public:
	Image();
	Image(const VkFormat& format, const VkImageUsageFlags& usage, const VkMemoryPropertyFlags& memoryProperties, const VkExtent2D& extent);

	// creates an Image referencing and existing VkImage
	Image(VkImage& image, VkFormat format, VkExtent2D extent);

	// creates an Image representing data in the texture file at path
	Image(const std::string& path, VkFormat textureFormat);
	~Image();

	void transitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

	// Copy buffer to this Image with no offset and with this Image's full extent
	void copyBufferToImage(VkBuffer buffer);
	void copyBufferToImage(const Buffer& buffer);

	void createLayoutBinding(int index, VkShaderStageFlags stage, VkDescriptorSetLayoutBinding& out) const override;
	void createDescriptorWrite(int index, const VkDescriptorSet& descriptorSet, VkWriteDescriptorSet& out) const override;


	// call sync functions (including getAttachmentDescription) in the order they will be executed in the command buffer. In the order they appear in the command buffer.
	void prepareForAccess(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier) override;
	void insertWARBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier) override;
	void insertRAWBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier) override;
	void insertFinalBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier) override;

	// returns attachment description and attachment reference for this Image, transitioning into the layout required by accessSpecifier. Returned via argument. Assumes layout transitions will be requested (get__Transition()) in the order they appear in the command buffer
	void getAttachmentDescription(AccessSpecifier currentAccess, AccessSpecifier futureAccess, VkAttachmentDescription& attachmentDescription, VkAttachmentReference& attachmentReference);


	VkImage& getImage();
	VkImageView& getImageView();
	VkSampler& getSampler();
	const VkFormat& getFormat();
	const VkExtent2D& getExtent();


private:

	static const std::map<AccessSpecifier::OPERATION, VkImageLayout> REQUIRED_LAYOUTS;

	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkSampler sampler;
	VkFormat format;
	VkImageUsageFlags usage;
	VkMemoryPropertyFlags memoryProperties;
	VkExtent2D extent;

	// Some Image objects reference an image created elswhere
	// TODO consider making them separate subclasses
	bool responsibleForImageDestruction = false;

	VkImageLayout currentLayout;
	

	void init(const VkFormat& format, const VkImageUsageFlags& usage, const VkMemoryPropertyFlags& memoryProperties, const VkExtent2D& extent);
};

