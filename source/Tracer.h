#pragma once

#include <vector>
#include "Scene.h"
#include "CameraController.h"
#include "GPUStructs.h"
#include "BVHNode.h"
#include "DrawPass.h"

class Tracer : public Scene
{
public:
	Tracer();
	Tracer(const std::string& filename);
	~Tracer();

	void initialize() override;
	Image& getPresentedImage() const override;
	void update() override;
	void run() override;
	
	CameraController& getCameraController();
	const std::vector<Sphere>& getSpheres() const;
	const std::vector<glm::vec4>& getVertPositions() const;
	const std::vector<glm::vec4>& getVertNormals() const;
	const std::vector<glm::vec4>& getVertUVCoords() const;
	const std::vector<Triangle>& getTriangles() const;
	const std::vector<BVHNodeStruct>& getBVHNodes() const;
	uint32_t getNumTriangles() const;
	AABB& getLightBox() const;
	float getLightArea() const;

private:





	const glm::vec3 bottomColor = { 0, 0, 0 };
	const glm::vec3 topColor = { 0, 0, 0 };

	const std::vector<Vertex> vertices =
	{
		{{-1, -1}, topColor, {0, 0}}, // TL
		{{-1, 1}, bottomColor, {0, 1}},  // BL
		{{1, 1}, bottomColor, {1, 1}},   // BR
		{{1, -1}, topColor, {1, 0}}   // TR
	};

	const std::vector<uint16_t> indices =
	{
		0, 3, 2, 2, 1, 0
	};







	const Material defaultTriangleMaterial =
	{
		glm::vec4(0.45f, 0.45f, 0.45f, 1),
		glm::vec4(0,0,0,0),
		0.0f,
		0.0f,
		0.0f
	};

	CameraController cameraController;

	std::vector<Sphere> spheres;
	std::vector<glm::vec4> vertPositions;
	std::vector<glm::vec4> vertNormals;
	std::vector<glm::vec4> vertUVCoords;
	std::vector<Triangle> triangles;
	BVHNode* BVH;
	std::vector<BVHNodeStruct> BVHNodes;
	uint32_t numTriangles = 0;

	AABB* lightBox;
	float lightArea;


	Buffer* vertexBuffer = nullptr;

	Buffer* indexBuffer = nullptr;
	Buffer* uniformBuffer = nullptr;
	Buffer* shaderStorageBuffer = nullptr;

	Image* renderImage = nullptr;
	Image* textureImage = nullptr;

	DrawPass* passRender = nullptr;
	std::map<Pass*, std::vector<Pass*>> predecessors;
	std::map<Pass*, std::vector<Pass*>> successors;

	void createVertexBuffer();
	void createIndexBuffer();
	void createRenderImage();
	void createUniformBuffers();

	

	void declarePasses();
	void preparePasses();

	void populateSpheres();
	void updateUniformBuffer();
	void loadModel(const std::string& filename);

	void populateShaderStorageBuffer();
};

