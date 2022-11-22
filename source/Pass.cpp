#include "Pass.h"

#include <array>
#include <fstream>
#include "GPUStructs.h"

Pass::Pass(std::vector<ResourceAccessSpecifier> descriptors) : descriptors(descriptors)
{
	// construct vector
	//this->descriptors = std::vector<Resource*>(numDescriptors);
	//for (int i = 0; i < numDescriptors; i++)
	//{
	//	this->descriptors.at(i) = descriptors[i];
	//}

	// create fence
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if (vkCreateFence(VulkanCoreSupport::getInstance().getDevice(), &fenceInfo, nullptr, &notExecuting) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor fence");
	}

	// register fence with descriptors
	for (ResourceAccessSpecifier resourceAccess : descriptors)
	{
		resourceAccess.resource->registerResourceUse(notExecuting);
	}

	createDescriptorSetLayout();
	createDescriptorSet();
	//createPipelineLayout();
	//createGraphicsPipeline();
	allocateCommandBuffer();
}

Pass::~Pass()
{
	VkDevice& device = VulkanCoreSupport::getInstance().getDevice();

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	//vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	//vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkFreeCommandBuffers(device, VulkanCoreSupport::getInstance().commandPool, 1, &commandBuffer);
}

void Pass::prepareExecution(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors)
{

}

void Pass::execute()
{
	// set this pass to executing
	vkResetFences(VulkanCoreSupport::getInstance().getDevice(), 1, &notExecuting);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;// waitSemaphores;
	submitInfo.pWaitDstStageMask = nullptr;// waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	if (vkQueueSubmit(VulkanCoreSupport::getInstance().graphicsQueue, 1, &submitInfo, notExecuting) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer");
	}
}





// User-specification
void Pass::createDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(descriptors.size());
	for (int i = 0; i < bindings.size(); i++)
	{
		descriptors.at(i).resource->createLayoutBinding(i, VK_SHADER_STAGE_FRAGMENT_BIT, bindings.at(i));
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(VulkanCoreSupport::getInstance().getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout");
	}
}

void Pass::createDescriptorSet()
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = VulkanCoreSupport::getInstance().descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(VulkanCoreSupport::getInstance().getDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets");
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites(descriptors.size());
	for (int i = 0; i < descriptors.size(); i++)
	{

		descriptors.at(i).resource->createDescriptorWrite(i, descriptorSet, descriptorWrites.at(i));
	}

	//vkUpdateDescriptorSets(VulkanCoreSupport::getInstance().getDevice(), static_cast<uint32_t>(descriptors.size()), descriptorWrites.data(), 0, nullptr);
}


static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to load shader files");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void Pass::allocateCommandBuffer()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VulkanCoreSupport::getInstance().commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(VulkanCoreSupport::getInstance().getDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}
}

// assume that the command buffer is not in use (in the initial state, in spec terms). Expects caller to handle that synchronization
void Pass::startCommandBufferRecording(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors)
{
	vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	// insert barriers

	// get the resources used in this Pass
	std::vector<ResourceAccessSpecifier> dstReads;
	getReadOnlyResources(dstReads);
	std::vector<ResourceAccessSpecifier> dstWrites;
	getWriteResources(dstWrites);

	for (Pass* srcPass : predecessors)
	{
		// get the resources used in that this Pass depends on
		std::vector<ResourceAccessSpecifier> srcReads;
		srcPass->getReadOnlyResources(srcReads);
		std::vector<ResourceAccessSpecifier> srcWrites;
		srcPass ->getWriteResources(srcWrites);

		// look for resources used in both passes

		// WAR dependencies
		for (ResourceAccessSpecifier srcAccess : srcReads)
		{
			for (ResourceAccessSpecifier dstAccess : dstWrites)
			{
				if (srcAccess.resource == dstAccess.resource)
				{
					// this Pass writes to dstAccess after srcPass reads it
					// insert barrier
					dstAccess.resource->insertWARBarrier(commandBuffer, dstAccess.accessSpecifier);
				}
			}
		}

		// RAW dependencies
		for (ResourceAccessSpecifier srcAccess : srcWrites)
		{
			for (ResourceAccessSpecifier dstAccess : dstReads)
			{
				if (srcAccess.resource == dstAccess.resource)
				{
					// this Pass reads from resource after dstPass writes to it
					// insert barrier
					dstAccess.resource->insertRAWBarrier(commandBuffer, dstAccess.accessSpecifier);
				}
			}
		}
	}
}

void Pass::endCommandBufferRecording()
{
	for (ResourceAccessSpecifier resourceAccessSpecifier : descriptors)
	{
		resourceAccessSpecifier.resource->insertFinalBarrier(commandBuffer, resourceAccessSpecifier.accessSpecifier);
	}

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer");
	}
}


VkDescriptorSet& Pass::getDescriptorSet()
{
	return descriptorSet;
}

//VkPipelineLayout& Pass::getPipelineLayout()
//{
//	return pipelineLayout;
//}
//
//VkPipeline& Pass::getGraphicsPipeline()
//{
//	return graphicsPipeline;
//}

VkCommandBuffer& Pass::getCommandBuffer()
{
	return commandBuffer;
}

//std::vector<Resource*>& Pass::getResources()
//{
//	return descriptors;
//}


void Pass::getReadOnlyResources(std::vector<ResourceAccessSpecifier>& output)
{
	output.clear();

	// descriptors
	for (ResourceAccessSpecifier descriptor : descriptors)
	{
		if (descriptor.accessSpecifier.accessType == AccessSpecifier::ACCESS_TYPE::READ)
		{
			output.push_back(descriptor);
		}
	}
}

void Pass::getWriteResources(std::vector<ResourceAccessSpecifier>& output)
{
	output.clear();

	// descriptors
	for (ResourceAccessSpecifier descriptor : descriptors)
	{
		if (descriptor.accessSpecifier.accessType == AccessSpecifier::ACCESS_TYPE::WRITE)
		{
			output.push_back(descriptor);
		}
	}
}