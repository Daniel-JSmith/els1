#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>
#include <iostream>





class VulkanCoreSupport
{
public:
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value()
				&& presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	
	static VulkanCoreSupport& getInstance();
	VkDevice& getDevice();
	VkPhysicalDevice& getPhysicalDevice();

	
	~VulkanCoreSupport();

	#ifdef NDEBUG
		const bool enableValidationLayers = false;
	#else
		const bool enableValidationLayers = true;
	#endif

	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_LUNARG_monitor"
	};

	const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		"VK_KHR_timeline_semaphore"
	};

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);





//private:
	VulkanCoreSupport();

	static VulkanCoreSupport instance;

	

	void pickPhysicalDevice();
	void printPhysicalDeviceProperties(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();


	

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	std::vector<const char*> getRequiredExtensions();

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	void createSwapChain();
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance
		, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
		, const VkAllocationCallbacks* pAllocator
		, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance
		, VkDebugUtilsMessengerEXT debugMessenger
		, const VkAllocationCallbacks* pAllocator);

	void createInstance();

	bool checkValidationLayerSupport();

	void createSurface();
	void initWindow();

	void createCommandPool();

	void createDescriptorPool();
	

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	VkInstance vInstance;

	VkSurfaceKHR surface;
	GLFWwindow* window;




	VkSwapchainKHR swapChain;
	//std::vector<VkImage> swapChainImages;
	//std::vector<VkImageView> swapChainImageViews;
	//std::vector<VkFramebuffer> swapChainFramebuffers;
	uint32_t numSwapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	//void createSyncObjects();
	//std::vector<VkFence> inFlightFences;
	//size_t currentFrame = 0;
	//std::vector<VkSemaphore> imageAvailableSemaphores;
	//std::vector<VkSemaphore> renderFinishedSemaphores;
	



	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkCommandPool commandPool;

	VkDescriptorPool descriptorPool;

	VkDebugUtilsMessengerEXT debugMessenger;
};

