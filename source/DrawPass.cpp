#include "DrawPass.h"

#include <array>
#include <fstream>
#include "GPUStructs.h"

DrawPass::DrawPass(Image*& outputAttachment, std::vector<ResourceAccessSpecifier> descriptors, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, Buffer& indexBuffer, Buffer& vertexBuffer) : Pass(descriptors), outputAttachment(outputAttachment), indexBuffer(indexBuffer), vertexBuffer(vertexBuffer)
{
	vertexShader = new Shader(vertexShaderPath);
	fragmentShader = new Shader(fragmentShaderPath);

	createPipelineLayout();
}

DrawPass::~DrawPass()
{
	VkDevice& device = VulkanCoreSupport::getInstance().getDevice();
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyFramebuffer(device, frameBuffer, nullptr);

	delete vertexShader;
	delete fragmentShader;
}

void DrawPass::prepareExecution(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors)
{
	Pass::prepareExecution(predecessors, successors);
	createRenderPass(predecessors, successors);
	createFrameBuffer();
	createGraphicsPipeline();
	recordCommandBuffer(predecessors, successors);
}

void DrawPass::createPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = VK_NULL_HANDLE;

	if (vkCreatePipelineLayout(VulkanCoreSupport::getInstance().getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout");
	}
}

void DrawPass::createRenderPass(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors)
{
	VkAttachmentDescription colorAttachment;
	VkAttachmentReference colorAttachmentRef;

	// Check if output attachment is used in descriptors
	// TODO code duplication
	// if no future access, assume prepare for presentation
	AccessSpecifier futureAccessSpecifier = AccessSpecifier{AccessSpecifier::ACCESS_TYPE::READ, AccessSpecifier::OPERATION::PREPARE_FOR_PRESENTATION};

	for (Pass* futurePass : successors)
	{
		// Get reads and writes of futurePass
		std::vector<ResourceAccessSpecifier> futureAccesses;
		futurePass->getReadOnlyResources(futureAccesses);
		std::vector<ResourceAccessSpecifier> futureWrites;
		futurePass->getWriteResources(futureWrites);
		futureAccesses.reserve(futureAccesses.size() + futureWrites.size());
		futureAccesses.insert(futureAccesses.end(), futureWrites.begin(), futureWrites.end());

		for (ResourceAccessSpecifier access : futureAccesses)
		{
			if (access.resource == outputAttachment)
			{
				// futurePass accesses this Pass's output attachment
				futureAccessSpecifier = access.accessSpecifier;
			}
		}
	}

	outputAttachment->getAttachmentDescription(AccessSpecifier{ AccessSpecifier::ACCESS_TYPE::WRITE, AccessSpecifier::OPERATION::COLOR_ATTACHMENT_OUTPUT}, futureAccessSpecifier, colorAttachment, colorAttachmentRef);


	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	// TODO use subpass dependencies

	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = VK_NULL_HANDLE;

	if (vkCreateRenderPass(VulkanCoreSupport::getInstance().getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass");
	}


}

void DrawPass::createFrameBuffer()
{
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = &outputAttachment->getImageView();
	framebufferInfo.width = outputAttachment->getExtent().width;
	framebufferInfo.height = outputAttachment->getExtent().height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(VulkanCoreSupport::getInstance().getDevice(), &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create frambuffer");
	}
}

void DrawPass::createGraphicsPipeline()
{
	VkExtent2D extent = outputAttachment->getExtent();

	// vertex shader
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertexShader->getModule();
	vertShaderStageInfo.pName = "main";

	// fragment shader
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragmentShader->getModule();
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// vertex shader input
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// scissor
	VkRect2D scissor{};
	scissor.offset = { 0,0 };
	scissor.extent = extent;

	// viewport state
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// rasterzation state
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	// multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	// color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT
		| VK_COLOR_COMPONENT_G_BIT
		| VK_COLOR_COMPONENT_B_BIT
		| VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; //VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA; //VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;


	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;

	pipelineInfo.layout = pipelineLayout;

	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(VulkanCoreSupport::getInstance().getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline");
	}
}

void DrawPass::recordCommandBuffer(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors)
{
	startCommandBufferRecording(predecessors, successors);

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = frameBuffer;
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = RenderParameters::RENDER_RESOLUTION;

	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	renderPassInfo.clearValueCount = 0;
	renderPassInfo.pClearValues = VK_NULL_HANDLE;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	VkBuffer vertexBuffers[] = { vertexBuffer.getBufferObject() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	// TODO Remove magic numbers. Create a class to specify vertices and how they should be read e.g. GeometryContainer. Include vertices, indices, index size, winding order
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getBufferObject(), 0, VK_INDEX_TYPE_UINT16);
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indexBuffer.size / 2), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	endCommandBufferRecording();
}

VkPipelineLayout& DrawPass::getPipelineLayout()
{
	return pipelineLayout;
}

VkPipeline& DrawPass::getGraphicsPipeline()
{
	return graphicsPipeline;
}