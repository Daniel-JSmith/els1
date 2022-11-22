#include "Buffer.h"
#include <stdexcept>

const std::map<Resource::ACCESS_PROPERTY, void (Buffer::*)(VkDeviceSize, const void*)> Buffer::DATA_TRANSFER_FUNCTIONS =
{
	{Resource::ACCESS_PROPERTY::CPU_PREFERRED, &copyDataDirect},
	{Resource::ACCESS_PROPERTY::GPU_PREFERRED, &copyDataStaging}
};

const std::map<Resource::ACCESS_PROPERTY, VkBufferUsageFlags> Buffer::ACCESS_PROPERTY_USAGE_FLAGS =
{
	{Resource::ACCESS_PROPERTY::CPU_PREFERRED, 0},
	{Resource::ACCESS_PROPERTY::GPU_PREFERRED, VK_BUFFER_USAGE_TRANSFER_DST_BIT}
};

const std::map<Resource::USE, VkBufferUsageFlags> Buffer::USE_USAGE_FLAGS =
{
	{Resource::USE::VERTEX_BUFFER, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
	{Resource::USE::INDEX_BUFFER, VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
	{Resource::USE::UNIFORM_BUFFER, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT},
	{Resource::USE::SHADER_STORAGE_BUFFER, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT},
	{Resource::USE::TRANSFER_SOURCE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT},
	{Resource::USE::TRANSFER_DESTINATION, VK_BUFFER_USAGE_TRANSFER_DST_BIT},
};

VkBufferUsageFlags Buffer::getUsageFlags()
{
	return ACCESS_PROPERTY_USAGE_FLAGS.at(accessProperty) | USE_USAGE_FLAGS.at(use);
}

void Buffer::copyDataDirect(VkDeviceSize bufferSize, const void* data)
{
	void* destinationData;
	vkMapMemory(VulkanCoreSupport::getInstance().getDevice(), bufferMemory, 0, bufferSize, 0, &destinationData);
	memcpy(destinationData, data, (size_t)bufferSize);
	vkUnmapMemory(VulkanCoreSupport::getInstance().getDevice(), bufferMemory);
}

void Buffer::copyDataStaging(VkDeviceSize bufferSize, const void* data)
{
	// create staging buffer, copy data to staging buffer, copy staging buffer to this buffer, delete staging buffer
	// Might be more performant to store a staging buffer as a member variable. I suspect that approach wouldn't be useful because the fact that the user is using a staging buffer implies that cpu -> gpu transfers are infrequent

	// create temp staging buffer
	// staging buffers aren't descriptors, so descriptor type is meaningless
	Buffer stagingBuffer(size, USE::TRANSFER_SOURCE, ACCESS_PROPERTY::CPU_PREFERRED, data);

	// copy data to GPU buffer
	copyBuffer(stagingBuffer);
}

void Buffer::setDataTransferFunction()
{
	dataTransferFunction = DATA_TRANSFER_FUNCTIONS.at(accessProperty);
}

void Buffer::copyData(VkDeviceSize bufferSize, const void* data)
{
	waitForReady();

	(*this.*dataTransferFunction)(bufferSize, data);
}

void Buffer::copyBuffer(const Buffer& otherBuffer)
{
	VkCommandBuffer commandBuffer = VulkanCoreSupport::getInstance().beginSingleTimeCommands();
	VkBufferCopy copyRegion{};
	copyRegion.size = otherBuffer.size;
	vkCmdCopyBuffer(commandBuffer, otherBuffer.bufferObject, bufferObject, 1, &copyRegion);
	VulkanCoreSupport::getInstance().endSingleTimeCommands(commandBuffer);
}

//// This constructor is used for staging buffers. There not really descriptors, so set descriptor type to anything
//Buffer::Buffer(VkDeviceSize bufferSize, const void* data) : size(bufferSize), Resource(VK_DESCRIPTOR_TYPE_SAMPLER, ACCESS_PROPERTY::CPU_PREFERRED)
//{
//	setDataTransferFunction();
//	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
//
//
//	// copty data to temp staging buffer
//	copyData(bufferSize, data);
//};


// Copies to staging buffer before populating the newly created buffer
Buffer::Buffer(VkDeviceSize size, USE use, ACCESS_PROPERTY accessProperty, const void* data) : size(size), Resource(use, accessProperty)
{
	setDataTransferFunction();
	createBuffer(size);

	copyData(size, data);
}


Buffer::Buffer(VkDeviceSize size, USE use, ACCESS_PROPERTY accessProperty) : size(size), Resource(use, accessProperty)
{
	setDataTransferFunction();
	createBuffer(size);
}

Buffer::~Buffer()
{
	vkDestroyBuffer(VulkanCoreSupport::getInstance().getDevice(), bufferObject, nullptr);
	vkFreeMemory(VulkanCoreSupport::getInstance().getDevice(), bufferMemory, nullptr);
};


const VkBuffer& Buffer::getBufferObject() const
{
	return bufferObject;
}

// TODO behavior on non-descriptor resources
void Buffer::createLayoutBinding(int index, VkShaderStageFlags stage, VkDescriptorSetLayoutBinding& out) const
{
	out.binding = index;
	out.descriptorType = DESCRIPTOR_TYPES.at(use);
	out.descriptorCount = 1;
	out.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	out.pImmutableSamplers = nullptr;
}

void Buffer::createDescriptorWrite(int index, const VkDescriptorSet& descriptorSet, VkWriteDescriptorSet& out) const
{
	VkDescriptorBufferInfo descriptorBufferInfo{};
	descriptorBufferInfo.buffer = bufferObject;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = size;

	out.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	out.dstSet = descriptorSet;
	out.dstBinding = index;
	out.dstArrayElement = 0;
	out.descriptorType = DESCRIPTOR_TYPES.at(use);
	out.descriptorCount = 1;
	out.pBufferInfo = &descriptorBufferInfo;
	out.pImageInfo = nullptr;
	out.pTexelBufferView = nullptr;
	out.pNext = nullptr;

	vkUpdateDescriptorSets(VulkanCoreSupport::getInstance().getDevice(), 1, &out, 0, nullptr);
}

void Buffer::createBuffer(VkDeviceSize size)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = getUsageFlags();
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(VulkanCoreSupport::getInstance().getDevice(), &bufferInfo, nullptr, &bufferObject) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(VulkanCoreSupport::getInstance().getDevice(), bufferObject, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;

	allocInfo.memoryTypeIndex = VulkanCoreSupport::getInstance().findMemoryType(memRequirements.memoryTypeBits, MEMORY_PROPERTY_FLAGS.at(accessProperty));

	if (vkAllocateMemory(VulkanCoreSupport::getInstance().getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory");
	}

	vkBindBufferMemory(VulkanCoreSupport::getInstance().getDevice(), bufferObject, bufferMemory, 0);
}

void Buffer::prepareForAccess(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier)
{

}

void Buffer::insertWARBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier)
{
	VkMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		0,
		1, &barrier,
		0, VK_NULL_HANDLE,
		0, VK_NULL_HANDLE
	);
}

void Buffer::insertRAWBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier)
{
	VkMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		0,
		1, &barrier,
		0, VK_NULL_HANDLE,
		0, VK_NULL_HANDLE
	);
}

void Buffer::insertFinalBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier)
{

}
