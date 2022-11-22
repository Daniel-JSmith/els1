#include "Tracer.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// TODO consider where this information should live
VkFormat renderTargetFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
VkFormat textureFormat = VK_FORMAT_R8G8B8A8_SRGB;
std::string filename = "Assets/Scenes/boxes.obj";

// load scene from a .obj file
Tracer::Tracer()
{
	populateSpheres();
	loadModel(filename);

	BVH = new BVHNode(vertPositions, triangles, 0, numTriangles);
	BVHNodes.resize(SIZE_BVH_ARR);
	BVH->toStructArray(BVHNodes);

	// point to triangles that emit light
	std::vector<Triangle> emittingTriangles;

	for (size_t i = 0; i < triangles.size(); i++)
	{
		if (glm::dot(triangles[i].material.emitted, glm::vec4(1)) > 0.0f)
		{
			emittingTriangles.push_back(triangles[i]);
		}
	}

	lightBox = new AABB(vertPositions, emittingTriangles, 0, emittingTriangles.size());

	glm::vec4 cornerMin = glm::vec4(lightBox->getMinCorner(), 0);
	glm::vec4 cornerMax = glm::vec4(lightBox->getMaxCorner(), 0);

	float xLength = (cornerMax.x - cornerMin.x);
	float yLength = (cornerMax.y - cornerMin.y);
	float zLength = (cornerMax.z - cornerMin.z);

	lightArea = (xLength * yLength + xLength * zLength + yLength * zLength);


	createRenderImage();

	createUniformBuffers();
	populateShaderStorageBuffer();

	createVertexBuffer();
	createIndexBuffer();

	declarePasses();
	preparePasses();
}

Tracer::~Tracer()
{
	delete lightBox;
	delete BVH;

	delete renderImage;
	delete textureImage;

	delete passRender;

	delete uniformBuffer;

	delete indexBuffer;
	delete vertexBuffer;
	delete shaderStorageBuffer;
}

void Tracer::initialize()
{

}

void Tracer::declarePasses()
{
	passRender = new DrawPass(renderImage, { ResourceAccessSpecifier{uniformBuffer, {AccessSpecifier::ACCESS_TYPE::READ}}, ResourceAccessSpecifier{shaderStorageBuffer, AccessSpecifier::ACCESS_TYPE::READ} }, "Assets/Shaders/simpleShader.vert.spv", "Assets/Shaders/simpleShader.frag.spv", *indexBuffer, *vertexBuffer);

	predecessors.insert(std::pair<Pass*, std::vector<Pass*>>(passRender, {}));
	successors.insert(std::pair<Pass*, std::vector<Pass*>>(passRender, {}));
}

void Tracer::preparePasses()
{
	// TODO Object->method(Object) OOP
	passRender->prepareExecution(predecessors.at(passRender), successors.at(passRender));
}

void Tracer::createRenderImage()
{
	renderImage = new Image(renderTargetFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, RenderParameters::RENDER_RESOLUTION);
}

void Tracer::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	vertexBuffer = new Buffer(bufferSize, Resource::USE::VERTEX_BUFFER, Resource::ACCESS_PROPERTY::GPU_PREFERRED, vertices.data());
}

void Tracer::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	indexBuffer = new Buffer(bufferSize, Resource::USE::INDEX_BUFFER, Resource::ACCESS_PROPERTY::GPU_PREFERRED, indices.data());
}

void Tracer::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	// TODO VK_MEMORY_PROPERTY_HOST_CACHED_BIT might improve performance
	uniformBuffer = new Buffer(bufferSize, Resource::USE::UNIFORM_BUFFER, Resource::ACCESS_PROPERTY::CPU_PREFERRED);
}


Image& Tracer::getPresentedImage() const
{
	return *renderImage;
}

void Tracer::update()
{
	cameraController.update();
	updateUniformBuffer();

	//srand(time(nullptr));
}

void Tracer::run()
{
	passRender->execute();
}

CameraController& Tracer::getCameraController()
{
	return cameraController;
}

const std::vector<Sphere>& Tracer::getSpheres() const
{
	return spheres;
}

const std::vector<glm::vec4>& Tracer::getVertPositions() const
{
	return vertPositions;
}

const std::vector<glm::vec4>& Tracer::getVertNormals() const
{
	return vertNormals;
}

const std::vector<glm::vec4>& Tracer::getVertUVCoords() const
{
	return vertUVCoords;
}

const std::vector<Triangle>& Tracer::getTriangles() const
{
	return triangles;
}

const std::vector<BVHNodeStruct>& Tracer::getBVHNodes() const
{
	return BVHNodes;
}

uint32_t Tracer::getNumTriangles() const
{
	return numTriangles;
}

AABB& Tracer::getLightBox() const
{
	return *lightBox;
}

float Tracer::getLightArea() const
{
	return lightArea;
}

void Tracer::populateSpheres()
{
	spheres.resize(NUM_PRIMITIVES);
	// ground
	spheres[0].position = glm::vec4(0, -100.5, -1, 0);
	spheres[0].radius = 0;// 100;
	spheres[0].material.color = glm::vec4(0.70f, 0.70f, 0.70f, 1);
	spheres[0].material.specularity = 0.0f;
	spheres[0].material.reflectance = 1.0f;
	spheres[0].material.refractiveIndex = 0.0f;

	// center
	spheres[1].position = glm::vec4(0, 0, -2, 0);
	spheres[1].radius = 0.25;
	spheres[1].material.color = glm::vec4(0.7f, 0.3f, 0.3f, 1);
	//spheres[1].material.emitted = glm::vec4(4, 4, 4, 1);
	spheres[1].material.emitted = glm::vec4(0, 0, 0, 1);
	spheres[1].material.specularity = 0;
	spheres[1].material.reflectance = 1.0f;
	spheres[1].material.refractiveIndex = 0.0f;

	// left
	spheres[2].position = glm::vec4(-1.5, 0, -2, 0);
	spheres[2].radius = 0.5f;
	spheres[2].material.color = glm::vec4(1.0f, 1.0f, 1.0f, 1);
	spheres[2].material.specularity = 01.0f;
	spheres[2].material.reflectance = 0.0f;
	spheres[2].material.refractiveIndex = 1.7f;

	// right
	spheres[3].position = glm::vec4(1.5, 0, -2, 0);
	spheres[3].radius = 0.5f;
	spheres[3].material.color = glm::vec4(0.80f, 0.40f, 0.20f, 1);
	spheres[3].material.specularity = 0.90f;
	spheres[3].material.reflectance = 1.0f;
	spheres[3].material.refractiveIndex = 0.0f;

	spheres[4].position = glm::vec4(0, 1, -7, 0);
	spheres[4].radius = 0;//1.5f;
	spheres[4].material.color = glm::vec4(1.0f, 1.0f, 1.0f, 1);
	spheres[4].material.specularity = 1.0f;
	spheres[4].material.reflectance = 1.0f;
	spheres[4].material.refractiveIndex = 0.0f;

	spheres[5].position = glm::vec4(0, 1, 3, 0);
	spheres[5].radius = 0;//1.5f;
	spheres[5].material.color = glm::vec4(0.70f, 0.50f, 0.60f, 1);
	spheres[5].material.specularity = 0.70f;
	spheres[5].material.reflectance = 0.50f;
	spheres[5].material.refractiveIndex = 0.0f;

	spheres[6].position = glm::vec4(-5, 1, -2, 0);
	spheres[6].radius = 0;//1.5f;
	spheres[6].material.color = glm::vec4(0.40f, 0.10f, 0.80f, 1);
	spheres[6].material.specularity = 0.0f;
	spheres[6].material.reflectance = 1.0f;
	spheres[6].material.refractiveIndex = 0.0f;
}

void copyToVec(const std::vector<float>& input, std::vector<glm::vec4>& output, int stride)
{
	for (size_t i = 0; (i + stride - 1) < input.size(); i += stride)
	{
		output.push_back(glm::vec4(input.at(i), stride >= 2 ? input.at(i + 1) : 0, stride >= 3 ? input.at(i + 2) : 0, 0));
	}
}

void convertObjMaterialToLocalFormat(tinyobj::material_t& input, Material& output)
{
	output.color = glm::vec4(input.diffuse[0], input.diffuse[1], input.diffuse[2], 0);
	output.emitted = glm::vec4(input.emission[0], input.emission[1], input.emission[2], 0);
	output.refractiveIndex = 0.0f;
	output.reflectance = 0.0f;
	output.specularity = 0.0f;// 1 - input.roughness;
}

void Tracer::loadModel(const std::string& filename)
{
	vertPositions.clear();
	triangles.clear();
	vertNormals.clear();
	vertUVCoords.clear();


	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning, error;

	std::map<std::string, int> materialMap;

	if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, filename.c_str(), "Assets/Scenes/"))
	{
		throw std::runtime_error(warning + error);
	}




	copyToVec(attributes.vertices, vertPositions, 3);
	copyToVec(attributes.normals, vertNormals, 3);
	copyToVec(attributes.texcoords, vertUVCoords, 2);

	for (const auto& shape : shapes)
	{
		for (size_t i = 0; (i + 2) < shape.mesh.indices.size(); i += 3)
		{
			Triangle temp{};
			temp.v0 = shape.mesh.indices.at(i).vertex_index;
			temp.v1 = shape.mesh.indices.at(i + 1).vertex_index;
			temp.v2 = shape.mesh.indices.at(i + 2).vertex_index;

			temp.n0 = shape.mesh.indices.at(i).normal_index;
			temp.n1 = shape.mesh.indices.at(i + 1).normal_index;
			temp.n2 = shape.mesh.indices.at(i + 2).normal_index;

			temp.t0 = shape.mesh.indices.at(i).texcoord_index;
			temp.t1 = shape.mesh.indices.at(i + 1).texcoord_index;
			temp.t2 = shape.mesh.indices.at(i + 2).texcoord_index;


			int materialIndex = shape.mesh.material_ids.at(i / 3);

			if (materialIndex == -1)
			{
				temp.material = defaultTriangleMaterial;
			}
			else
			{
				convertObjMaterialToLocalFormat(materials.at(materialIndex), temp.material);
			}

			triangles.push_back(temp);
		}
	}

	numTriangles = triangles.size();
}

void Tracer::populateShaderStorageBuffer()
{
	// populate temp SSBO struct, copy that to GPU buffer, delete temp struct
	StorageBufferObject* ssbo = new StorageBufferObject{};

	// TODO remove calls
	std::copy(getSpheres().begin(), getSpheres().end(), ssbo->spheres);
	std::copy(getVertPositions().begin(), getVertPositions().end(), ssbo->vertPositions);
	std::copy(getVertNormals().begin(), getVertNormals().end(), ssbo->vertNormals);
	std::copy(getVertUVCoords().begin(), getVertUVCoords().end(), ssbo->vertUVCoords);
	std::copy(getTriangles().begin(), getTriangles().end(), ssbo->triangles);
	std::copy(getBVHNodes().begin(), getBVHNodes().end(), ssbo->BVHNodes);
	ssbo->numTriangles = getNumTriangles();

	ssbo->lightBVHNode.cornerMin = glm::vec4(getLightBox().getMinCorner(), 0);
	ssbo->lightBVHNode.cornerMax = glm::vec4(getLightBox().getMaxCorner(), 0);
	ssbo->lightArea = getLightArea();

	// copy to GPU buffer
	//VulkanCoreSupport::getInstance().populateBuffer(sizeof(*ssbo), ssbo, shaderStorageBuffer);

	shaderStorageBuffer = new Buffer(sizeof(*ssbo), Resource::USE::SHADER_STORAGE_BUFFER, Resource::ACCESS_PROPERTY::GPU_PREFERRED, ssbo);

	// delete temp struct
	delete ssbo;
	ssbo = nullptr;
}

void Tracer::updateUniformBuffer()
{
	UniformBufferObject ubo = UniformBufferObject{};

	ubo.RNGSeed = static_cast<uint32_t>(rand());
	ubo.camera = cameraController.getCamera1().getCameraStruct();

	uniformBuffer->copyData(sizeof(ubo), &ubo);
}