#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <map>
#include "VulkanCoreSupport.h"
#include "AccessSpecifier.h"

class Resource
{
public:

	// Indicates how memory should be transfered to GPU.
	// A resource with CPU_PREFERRED will use memory that allows quick cpu access.
	// A resource with GPU_PREFERRED will use memory that allows quick gpu access. Memory transfers will use staging buffer, resulting in overhead
	enum ACCESS_PROPERTY
	{
		CPU_PREFERRED,
		GPU_PREFERRED,
	};

	enum USE
	{
		ATTACHMENT_OUTPUT,
		SAMPLER,
		VERTEX_BUFFER,
		INDEX_BUFFER,
		UNIFORM_BUFFER,
		SHADER_STORAGE_BUFFER,
		TRANSFER_SOURCE,
		TRANSFER_DESTINATION
	};

	Resource(USE usage, ACCESS_PROPERTY accessProperty);

	~Resource();

	// returns a VkDescriptorSetLayoutBinding struct representing this Descriptor
	// index binding of the descriptor
	// stage which shader stage to bind the descriptor to
	// out return value
	virtual void createLayoutBinding(int index, VkShaderStageFlags stage, VkDescriptorSetLayoutBinding& out) const = 0;

	// returns a VkWriteDescriptorSet struct representing this Descriptor
	// index binding of the descriptor
	// out return value
	virtual void createDescriptorWrite(int index, const VkDescriptorSet& descriptorSet, VkWriteDescriptorSet& out) const = 0;

	// Prepare this Resource for the access described in accessSpecifier
	// May record commands to commandBuffer
	virtual void prepareForAccess(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier) = 0;

	// inserts a barrier to commandBUffer
	virtual void insertWARBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier) = 0;

	virtual void insertRAWBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier) = 0;

	// inserts any final cleanup (e.g. layout transitions) to commandBuffer
	virtual void insertFinalBarrier(VkCommandBuffer& commandBuffer, AccessSpecifier accessSpecifier) = 0;

	// Ensures that the host will not update this resource while passFance is unsignaled
	void registerResourceUse(const VkFence& passFence);

	

protected:


	static const std::map<ACCESS_PROPERTY, VkMemoryPropertyFlags> MEMORY_PROPERTY_FLAGS;
	static const std::map<USE, VkDescriptorType> DESCRIPTOR_TYPES;


	const USE use;


	// wait until this Resource is not in use
	virtual void waitForReady() const;

	const ACCESS_PROPERTY accessProperty;

private:
	// TODO switch to timeline semaphores
	// each elements represents whether a command buffer that uses this Resource is not in the queue. Used to synchronize host writes
	std::vector<VkFence> notInUseFences;
};