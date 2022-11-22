#pragma once

#include <algorithm>
#include <array>
#include <glm/glm.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "camera.h"

const int NUM_PRIMITIVES = 7;
const int VERTS_BUFFER_LENGTH = 5000;
const int TRIANGLES_BUFFER_LENGTH = 5000;
const int LIGHTS_BUFFER_LENGTH = 5;
const int SIZE_BVH_ARR = 30000;// (1 << NUM_PRIMITIVES) - 1;


struct Material
{
	alignas(16)glm::vec4 color;
	alignas(16)glm::vec4 emitted;
	alignas(4)float specularity; // expects 0 <= specularity <= 1
	alignas(4)float reflectance; // 0 <= reflectance <= 1. reflectance == 1 means always reflects; reflectance == 0 means always refracts
	alignas(4)float refractiveIndex;
};

struct Sphere
{
	alignas(16)glm::vec4 position;
	alignas(4)float radius; // indices of uniform array must be a size of some multiple of 16
	Material material;
};

struct Triangle // expects CW winding order
{
	// positions
	alignas(4)int v0;
	alignas(4)int v1;
	alignas(4)int v2;
	// normals
	alignas(4)int n0;
	alignas(4)int n1;
	alignas(4)int n2;
	// uv coordinates
	alignas(4)int t0;
	alignas(4)int t1;
	alignas(4)int t2;
	Material material;
};

struct AABox
{
	alignas(16)glm::vec4 corner1;
	alignas(16)glm::vec4 corner2;
	Material material;
};

struct BVHNodeStruct
{
	alignas(16)glm::vec4 cornerMin;
	alignas(16)glm::vec4 cornerMax;
	alignas(4)int primitiveIndex = -1;
};

struct UniformBufferObject
{
	alignas(4)uint32_t RNGSeed;
	CameraStruct camera;
};

struct StorageBufferObject
{
	Sphere spheres[NUM_PRIMITIVES];
	BVHNodeStruct BVHNodes[SIZE_BVH_ARR];
	alignas(16)glm::vec4 vertPositions[VERTS_BUFFER_LENGTH];
	alignas(16)glm::vec4 vertNormals[VERTS_BUFFER_LENGTH];
	alignas(16)glm::vec4 vertUVCoords[VERTS_BUFFER_LENGTH];
	Triangle triangles[TRIANGLES_BUFFER_LENGTH];
	alignas(4)uint32_t numTriangles;

	BVHNodeStruct lightBVHNode;
	alignas(4)float lightArea;
};

struct BlendingData
{
	alignas(4)int numRenderPasses;
};

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};