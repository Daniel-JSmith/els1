#pragma once
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Image.h"
#include "RenderParameters.h"
#include "VulkanCoreSupport.h"
#include "ResourceAccessSpecifier.h"

class Pass
{
public:
	Pass(std::vector<ResourceAccessSpecifier> descriptors);
	~Pass();

	// completes initialization required for executing this Pass
	virtual void prepareExecution(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors);

	// Submits command buffer to queue
	void execute();

	// TODO try to remove getters to Vulkan objects

	VkDescriptorSet& getDescriptorSet();
	//VkPipelineLayout& getPipelineLayout();
	//VkPipeline& getGraphicsPipeline();
	VkCommandBuffer& getCommandBuffer();
	VkImage& getImage();

	// returns vector to pointers of resources read in this Pass
	void getReadOnlyResources(std::vector<ResourceAccessSpecifier>& output);
	// returns vector to pointers of resources written to in this Pass
	void getWriteResources(std::vector<ResourceAccessSpecifier>& output);


	// TODO refactor access modifiers
protected:
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

	VkCommandBuffer commandBuffer;

	void startCommandBufferRecording(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors);
	void endCommandBufferRecording();
	virtual void recordCommandBuffer(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors) = 0;




private:
		
	//std::string vertFile;
	//std::string fragFile;
	
	

	std::vector<ResourceAccessSpecifier> descriptors;

	VkFence notExecuting;

	//VkPipelineLayout pipelineLayout;

	//VkPipeline graphicsPipeline;
	

	
	
	
	


	void createDescriptorSetLayout();
	void createDescriptorSet();
	//void createPipelineLayout();
	//void createGraphicsPipeline();
	void allocateCommandBuffer();
	
};