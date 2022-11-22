#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <map>
#include "VulkanCoreSupport.h"
#include "Resource.h"


class Buffer : public Resource
{
public:
	// TODO client should not have to specify both VkDescriptorType and VkBufferUsage. They mostly mean the same thing

	// Create buffer and fill with data
	Buffer(VkDeviceSize size, USE use, ACCESS_PROPERTY accessProperty, const void* data);

	// Create empty buffer
	Buffer(VkDeviceSize size, USE use, ACCESS_PROPERTY accessProperty);

	~Buffer();

	// TODO document what this is
	const VkDeviceSize size;

	const VkBuffer& getBufferObject() const;
	
	// TODO Copies data to buffer. Uses transfer technique appropriate for this Buffer objects accessProperty
	void copyData(VkDeviceSize bufferSize, const void* data);

	void createLayoutBinding(int index, VkShaderStageFlags stage, VkDescriptorSetLayoutBinding& out) const override;
	void createDescriptorWrite(int index, const VkDescriptorSet& descriptorSet, VkWriteDescriptorSet& out) const override;

	void prepareForAccess(VkCommandBuffer& commandBUffer, AccessSpecifier accessSpecifier) override;

	void insertWARBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier) override;
	void insertRAWBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier) override;
	void insertFinalBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier) override;

private:

	// TODO maybe gpu-preferred, cpu-preferred, staging should be different subclasses. I think that would be more readable

	static const std::map<ACCESS_PROPERTY, void (Buffer::*)(VkDeviceSize, const void*)> DATA_TRANSFER_FUNCTIONS;
	static const std::map<ACCESS_PROPERTY, VkBufferUsageFlags> ACCESS_PROPERTY_USAGE_FLAGS;
	static const std::map<USE, VkBufferUsageFlags> USE_USAGE_FLAGS;

	VkBuffer bufferObject;
	// TODO move memory to Resource superclass
	VkDeviceMemory bufferMemory;

	void (Buffer::*dataTransferFunction)(VkDeviceSize, const void*);

	void setDataTransferFunction();

	VkBufferUsageFlags getUsageFlags();

	void createBuffer(VkDeviceSize size);
	
	void copyBuffer(const Buffer& otherBuffer);

	void copyDataDirect(VkDeviceSize bufferSize, const void* data);
	void copyDataStaging(VkDeviceSize bufferSize, const void* data);
};