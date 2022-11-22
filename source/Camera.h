#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// camera representation the shader expects
struct CameraStruct
{
	alignas(16)glm::vec4 position;
	alignas(16)glm::vec4 up;
	alignas(16)glm::vec4 forward;
	alignas(16)glm::vec4 right;

	alignas(4)int pixelWidth;
	alignas(4)int pixelHeight;

	alignas(4)float focalPlaneWidth;
	alignas(4)float focalPlaneHeight;
	alignas(16)glm::vec4 lowerLeftCorner; // position;
};

// TODO does this stuff belong here?
const glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);
const float PI = 3.14f;
const float DEG_TO_RAD = PI / 180;

const float SENSITIVITY = 0.5f;


class Camera
{
private:
	
	glm::vec3 position;
	glm::vec3 forward;
	float verticalFov;
	int pixelWidth;
	int pixelHeight;
	float focusDistance;
	float focalPlaneHeight;
	float focalPlaneWidth;

public:
	Camera(glm::vec3 position, glm::vec3 forward, float verticalFov, int pixelWith, int pixelHeight);

	CameraStruct getCameraStruct() const;

	// moves this camera delta units along axis
	void offsetPosition(const glm::vec3& axis, float delta);

	// takes radians. applies sensitivity
	void rotate(const glm::vec3& axis, float radians);

	void lookAt(const glm::vec3& lookAt);

	const glm::vec3& getForward() const;
	// calculates right
	glm::vec3 getRight() const;
	const glm::vec3& getPosition() const;
};