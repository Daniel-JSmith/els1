#include "Resource.h"

const std::map<Resource::ACCESS_PROPERTY, VkMemoryPropertyFlags> Resource::MEMORY_PROPERTY_FLAGS =
{
	{Resource::ACCESS_PROPERTY::CPU_PREFERRED, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
	{Resource::ACCESS_PROPERTY::GPU_PREFERRED, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
};

const std::map<Resource::USE, VkDescriptorType> Resource::DESCRIPTOR_TYPES =
{
	{Resource::USE::SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER},
	{Resource::USE::UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
	{Resource::USE::SHADER_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
};

Resource::Resource(USE use, ACCESS_PROPERTY accessProperty) : use(use), accessProperty(accessProperty)
{

}

Resource::~Resource()
{

}

void Resource::registerResourceUse(const VkFence& passFence)
{
	notInUseFences.push_back(passFence);
}

void Resource::waitForReady() const
{
	if (notInUseFences.size() > 0)
	{
		if (vkWaitForFences(VulkanCoreSupport::getInstance().getDevice(), notInUseFences.size(), notInUseFences.data(), VK_TRUE, UINT64_MAX) != VK_SUCCESS)
		{
			throw std::runtime_error("wait for fence timeout");
		}
	}
}