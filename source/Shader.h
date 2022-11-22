#pragma once

#include <string>

#include "VulkanCoreSupport.h"

// A wrapper for a shader file and the associated shader object in the GPU API
class Shader
{
public:
	Shader(const std::vector<char>& code);
	Shader(const std::string& filename);
	~Shader();

	const VkShaderModule& getModule() const;

private:
	VkShaderModule shaderModule;
};