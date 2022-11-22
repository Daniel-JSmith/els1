#pragma once
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Image.h"
#include "RenderParameters.h"
#include "VulkanCoreSupport.h"
#include "ResourceAccessSpecifier.h"
#include "Pass.h"
#include "Shader.h"

class DrawPass : public Pass
{
public:
	DrawPass(Image*& outputAttachment, std::vector<ResourceAccessSpecifier> descriptors, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, Buffer& indexBuffer, Buffer& vertexBuffer);
	~DrawPass();

	void prepareExecution(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors) override;

	VkRenderPass& getRenderPass();
	VkFramebuffer& getFrameBuffer();
	VkPipelineLayout& getPipelineLayout();
	VkPipeline& getGraphicsPipeline();


private:
	Buffer& indexBuffer;
	Buffer& vertexBuffer;
	Shader* vertexShader;
	Shader* fragmentShader;
	Image*& outputAttachment;
	VkRenderPass renderPass;
	VkFramebuffer frameBuffer;

	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	void createPipelineLayout();
	void createGraphicsPipeline();
	void createRenderPass(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors);
	void createFrameBuffer();
	void recordCommandBuffer(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors) override;
};